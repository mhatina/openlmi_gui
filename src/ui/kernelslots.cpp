#include "kernel.h"

#include <gnome-keyring-1/gnome-keyring.h>
#include <QProcess>
#include <QStatusBar>
#include <QToolButton>

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;

int Engine::Kernel::getSilentConnection(std::string ip, bool silent)
{
    Logger::getInstance()->debug("Engine::Kernel::getSilentConnection(std::string ip)");
    if (m_connections.find(ip) == m_connections.end()) {
        CIMClient *client = NULL;
        GList *res_list;

        m_mutex->lock();
        GnomeKeyringAttributeList *list = gnome_keyring_attribute_list_new();        
        gnome_keyring_attribute_list_append_string(list, "server", ip.c_str());

        GnomeKeyringResult res = gnome_keyring_find_items_sync(
                    GNOME_KEYRING_ITEM_NETWORK_PASSWORD,
                    list,
                    &res_list
                    );
        m_mutex->unlock();

        if (res == GNOME_KEYRING_RESULT_NO_MATCH) {
            gnome_keyring_found_list_free(res_list);
            return 1;
        } else if (res != GNOME_KEYRING_RESULT_OK) {
            emit error("Cannot get username and password");
            gnome_keyring_found_list_free(res_list);
            return -1;
        }

        std::string username;
        m_mutex->lock();
        GnomeKeyringFound *keyring = ((GnomeKeyringFound*) res_list->data);
        guint cnt = g_array_get_element_size(keyring->attributes);
        for (guint i = 0; i < cnt; i++) {
            GnomeKeyringAttribute tmp;
            if (strcmp((tmp = g_array_index(keyring->attributes, GnomeKeyringAttribute, i)).name, "user") == 0) {
                username = gnome_keyring_attribute_get_string(&tmp);
                break;
            }
        }
        m_mutex->unlock();

        gchar* passwd;
        m_mutex->lock();
        gnome_keyring_find_password_sync(
                    GNOME_KEYRING_NETWORK_PASSWORD,
                    &passwd,
                    "user", username.c_str(),
                    "server", ip.c_str(),
                    NULL
                    );
        gnome_keyring_found_list_free(res_list);
        m_mutex->unlock();

        client = new CIMClient();
        try {
            try {
                // TODO settings
                client->setVerifyCertificate(false);
                client->connect(ip, 5989, true, username, passwd);
            } catch (Pegasus::Exception &ex) {
                Logger::getInstance()->info(std::string(ex.getMessage().getCString()) + " Trying another port.");
                client->connect(ip, 5988, false, username, passwd);
            }
            m_connections[ip] = client;
            return 0;
        } catch (Pegasus::Exception &ex) {
            if (!silent)
                emit error(std::string(ex.getMessage().getCString()));
            return -1;
        }
    }

    return 0;
}

void Engine::Kernel::deletePasswd()
{
    Logger::getInstance()->debug("Engine::Kernel::deletePasswd()");
    std::string id = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]->text(0).toStdString();
    deletePasswd(id);
}

void Engine::Kernel::deletePasswd(std::string id)
{
    Logger::getInstance()->debug("Engine::Kernel::deletePasswd(std::string id)");
    GnomeKeyringResult res = gnome_keyring_delete_password_sync(
                GNOME_KEYRING_NETWORK_PASSWORD,
                "server", id.c_str(),
                NULL
                );

    if (res != GNOME_KEYRING_RESULT_OK)
        Logger::getInstance()->error("Cannot delete password");
    else
        Logger::getInstance()->info("Password deleted");
}

void Engine::Kernel::emitSilentConnection(std::string ip)
{
    m_event_log->setConnectionStorage(&m_connections);
    boost::thread(boost::bind(&Engine::Kernel::getSilentConnection, this, ip, true));
}

void Engine::Kernel::enableSpecialButtons()
{
    Logger::getInstance()->debug("Engine::Kernel::enableSpecialButtons()");
    enableSpecialButtons(true);
}

void Engine::Kernel::enableSpecialButtons(bool state)
{
    Logger::getInstance()->debug("Engine::Kernel::enableSpecialButtons(bool state)");
    QList<QTreeWidgetItem*> list = m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    bool refresh = !list.empty() & m_refreshEnabled & state;
    QPushButton *button = m_main_window.findChild<QPushButton*>("refresh_button");
    button->setEnabled(refresh);
    button = m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button");
    button->setEnabled(refresh);
    m_main_window.getToolbar()->findChild<QToolButton*>("power_button")->setEnabled(refresh);
}

void Engine::Kernel::handleAuthentication(PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleAuthentication(PowerStateValues::POWER_VALUES state)");
    QTreeWidgetItem* item = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->text(0).toStdString();
    AuthenticationDialog dialog(ip);
    if (dialog.exec()) {
        std::string username = dialog.getUsername();
        std::string passwd = dialog.getPasswd();
        if (username == "" || passwd == "")
            return;

        GnomeKeyringResult res =
                gnome_keyring_store_password_sync(
                    GNOME_KEYRING_NETWORK_PASSWORD,
                    OPENLMI_KEYRING_DEFAULT,
                    ip.c_str(),
                    passwd.c_str(),
                    "user", username.c_str(),
                    "server", ip.c_str(),
                    NULL
                    );

        if (res != GNOME_KEYRING_RESULT_OK) {
            Logger::getInstance()->error("Cannot store password!");
            return;
        }

        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, state));
    } else {
        handleProgressState(1);
        m_main_window.getStatusBar()->clearMessage();
    }
}

void Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)");
    if (client == NULL) {
        handleProgressState(1);
        return;
    }

    if (state == PowerStateValues::NoPowerSetting) {
        QTabWidget* tab = m_main_window.getProviderWidget()->getTabWidget();
        IPlugin *plugin = (IPlugin*) tab->currentWidget();

        if (plugin != NULL) {
            // quick enough, maybe later move to another thread
            setMac(client);

            plugin->refresh(client);
            plugin->setSystemId(client->hostname());
            std::string title = WINDOW_TITLE;
            title += " @ " + client->hostname();
            m_main_window.setWindowTitle(title.c_str());
        }
    } else {
        setPowerState(client, state);
        handleProgressState(100);
    }
}

void Engine::Kernel::handleError(std::string message)
{
    Logger::getInstance()->debug("Engine::Kernel::handleError(std::string message)");
    m_main_window.getStatusBar()->clearMessage();
    Logger::getInstance()->error(message);
}

void Engine::Kernel::handleInstructionText(std::string text)
{
    Logger::getInstance()->debug("Engine::Kernel::handleInstructionText(std::string text)");
    m_code_dialog.setText(text, false);
}

void Engine::Kernel::handleProgressState(int state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleProgressState(int state)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    if (plugin == NULL)
        return;

    if (state == 100) {
        tab->setEnabled(true);
        plugin->setPluginEnabled(true);
        setButtonsEnabled(true);
//        m_main_window.getStatusBar()->clearMessage();
        m_bar->hide();
    } else if (state == 0) {
        plugin->setPluginEnabled(false);
        m_bar->setMaximum(0);
        m_bar->show();
    } else {
        tab->setEnabled(true);
        plugin->setPluginEnabled(false);
        enableSpecialButtons(true);
        m_bar->hide();
    }
}

void Engine::Kernel::refresh()
{
    Logger::getInstance()->debug("Engine::Kernel::refresh()");
    if (!m_refreshEnabled || m_main_window.getPcTreeWidget()->getTree()->selectedItems().empty())
        return;

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();

    if (plugin == NULL)
        return;

    tab->setEnabled(false);
    setButtonsEnabled(false);
    handleProgressState(0);
    boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::NoPowerSetting));
}

void Engine::Kernel::resetKeyring()
{
    Logger::getInstance()->debug("Engine::Kernel::resetKeyring()");
    GnomeKeyringResult res = gnome_keyring_delete_sync(OPENLMI_KEYRING_DEFAULT);
    if (res != GNOME_KEYRING_RESULT_OK) {
        Logger::getInstance()->error("Cannot reset keyring");
        return;
    }
    createKeyring();
}

void Engine::Kernel::saveAsScripts()
{
    Logger::getInstance()->debug("Engine::Kernel::saveAsScripts()");
    m_save_script_path = "";
    saveScripts();
}


void Engine::Kernel::saveScripts()
{
    Logger::getInstance()->debug("Engine::Kernel::saveScripts()");
    if (m_save_script_path.empty()) {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        m_save_script_path = dialog.getExistingDirectory().toStdString();
        m_save_script_path += "/" + m_main_window.getPcTreeWidget()->getTree()
                ->selectedItems()[0]->text(0).toStdString();
    }

    plugin_map::iterator it;
    for (it = m_loaded_plugins.begin(); it != m_loaded_plugins.end(); it++)
        it->second->saveScript(m_save_script_path);
}

void Engine::Kernel::setActivePlugin(int index)
{
    Logger::getInstance()->debug("Engine::Kernel::setActivePlugin(int index)");
    int i = 0;
    for (plugin_map::iterator it = m_loaded_plugins.begin(); it != m_loaded_plugins.end(); it++) {
        if (i == index) {
            (*it).second->setActive(true);
            m_code_dialog.setText((*it).second->getInstructionText(), false);
            setButtonsEnabled((*it).second->isRefreshed(), false);
            QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");
            if (button != NULL)
                button->setChecked((*it).second->isFilterShown());
            std::string title = WINDOW_TITLE;
            if (!(*it).second->getSystemId().empty())
                title += " @ " + (*it).second->getSystemId();
            m_main_window.setWindowTitle(title.c_str());
        } else {
            (*it).second->setActive(false);            
        }
        i++;
    }
}

void Engine::Kernel::setEditState(bool state)
{
    Logger::getInstance()->debug("Engine::Kernel::setEditState(bool state)");
    PCTreeWidget* tree_widget = m_main_window.getPcTreeWidget();
    m_refreshEnabled = !state;
    m_main_window.getProviderWidget()->setEnabled(!state);
    setButtonsEnabled(!state);
    bool empty = m_main_window.getPcTreeWidget()->getTree()->selectedItems().empty();
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button"))->setEnabled(state & !empty);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("add_button"))->setEnabled(state);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("remove_button"))->setEnabled(state);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("discover_button"))->setEnabled(state);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("edit_button"))->setIcon(
                QIcon(!state ? ":/changes-prevent.png" : ":/changes-allow.png"));

    tree_widget->setEditState(state);
}

void Engine::Kernel::setPluginNoChanges(IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::setPluginNoChanges(IPlugin *plugin)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setTabText(getIndexOfTab(plugin->getLabel()), plugin->getLabel().c_str());
}

void Engine::Kernel::setPluginUnsavedChanges(IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::setPluginUnsavedChanges(IPlugin *plugin)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setTabText(getIndexOfTab(plugin->getLabel()), std::string("* " + plugin->getLabel()).c_str());
}

void Engine::Kernel::setPowerState(QAction *action)
{
    Logger::getInstance()->debug("Engine::Kernel::setPowerState(QAction *action)");
    QTreeWidgetItem *item = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string message = "";
    handleProgressState(0);
    if (action->objectName().toStdString() == "reboot_action") {
        message = "Rebooting system: ";
        m_main_window.getPcTreeWidget()->setComputerIcon(QIcon(":/reboot.png"));
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerCycleOffSoft));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = "Shutting down system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerOffSoftGraceful));
    } else if (action->objectName().toStdString() == "force_reset_action") {
        message = "Force rebooting system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerCycleOffHard));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = "Force off system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerOffHard));
    } else if (action->objectName().toStdString() == "wake_on_lan") {
        message = "Waking system: ";
        wakeOnLan();
    }

    Logger::getInstance()->info(message + item->text(0).toStdString());
}


void Engine::Kernel::showCodeDialog()
{
    Logger::getInstance()->debug("Engine::Kernel::showCodeDialog()");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    if (plugin == NULL)
        return;
    m_code_dialog.setText(plugin->getInstructionText(), false);
    m_code_dialog.show();
}

void Engine::Kernel::showFilter()
{
    Logger::getInstance()->debug("Engine::Kernel::showFilter()");
    QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();

    if (plugin == NULL) {
        button->setChecked(false);
        return;
    }

    plugin->showFilter(button->isChecked());
}

void Engine::Kernel::startLMIShell()
{
    std::string command;
    QList<QTreeWidgetItem*> list = m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty())
        // TODO add to settings
        // TODO certificate
        command = "/bin/xterm -e \" lmishell";
    else {
        TreeWidgetItem *item = (TreeWidgetItem*) list[0];
        // TODO add to settings
        command = "/bin/xterm -e \" lmishell"
            + (!item->getIpv4().empty() ? item->getIpv4() : item->getIpv6()) + "\"";
    }
    QProcess *shell = new QProcess(this);
    shell->startDetached(command.c_str());
}

void Engine::Kernel::startSsh()
{
    QList<QTreeWidgetItem*> list = m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty())
        return;

    TreeWidgetItem *item = (TreeWidgetItem*) list[0];
    // TODO add to settings
    std::string command = "/bin/xterm -e \"ssh "
            + (!item->getIpv4().empty() ? item->getIpv4() : item->getIpv6()) + "\"";
    QProcess *shell = new QProcess(this);
    shell->startDetached(command.c_str());
}
