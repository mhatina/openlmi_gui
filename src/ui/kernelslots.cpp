#include "kernel.h"
#include "settingsdialog.h"

#include <gnome-keyring-1/gnome-keyring.h>
#include <QCheckBox>
#include <QLineEdit>
#include <QProcess>
#include <QStatusBar>
#include <QString>
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
        GnomeKeyringFound *keyring = ((GnomeKeyringFound *) res_list->data);
        guint cnt = g_array_get_element_size(keyring->attributes);
        for (guint i = 0; i < cnt; i++) {
            GnomeKeyringAttribute tmp;
            if (strcmp((tmp = g_array_index(keyring->attributes, GnomeKeyringAttribute,
                                            i)).name, "user") == 0) {
                username = gnome_keyring_attribute_get_string(&tmp);
                break;
            }
        }
        m_mutex->unlock();

        gchar *passwd;
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
            bool verify = false;
            std::string cert;

            try {
                verify = m_settings->value<bool, QCheckBox *>("use_certificate_checkbox");
                cert = m_settings->value<std::string, QLineEdit *>("certificate");

                client->setVerifyCertificate(verify);
                client->connect(ip, 5989, true, username, passwd, cert);
            } catch (Pegasus::Exception &ex) {
                Logger::getInstance()->info(std::string(ex.getMessage().getCString()) +
                                            " Trying another port.");
                client->setVerifyCertificate(verify);
                client->connect(ip, 5988, false, username, passwd, cert);
            }
            m_connections[ip] = client;
            return 0;
        } catch (Pegasus::Exception &ex) {
            if (!silent) {
                emit error(std::string(ex.getMessage().getCString()));
            }
            return -1;
        }
    }

    return 0;
}

void Engine::Kernel::deletePasswd()
{
    Logger::getInstance()->debug("Engine::Kernel::deletePasswd()");
    TreeWidgetItem *item = (TreeWidgetItem *)
                           m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string id = item->getId();
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

    if (res != GNOME_KEYRING_RESULT_OK) {
        Logger::getInstance()->info("Cannot delete password");
    } else {
        Logger::getInstance()->info("Password deleted");
    }
}

void Engine::Kernel::enableSpecialButtons(bool state)
{
    Logger::getInstance()->debug("Engine::Kernel::enableSpecialButtons(bool state)");
    QList<QTreeWidgetItem *> list =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    bool refresh = !list.empty() & m_refreshEnabled & state;
    QPushButton *button = widget<QPushButton *>("refresh_button");
    button->setEnabled(refresh);
    button = widget<QPushButton *>("delete_passwd_button");
    button->setEnabled(refresh);
    widget<QToolButton *>("power_button")->setEnabled(
        refresh);
}

void Engine::Kernel::handleAuthentication(PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleAuthentication(PowerStateValues::POWER_VALUES state)");
    TreeWidgetItem *item = (TreeWidgetItem *)
                           m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->getId();
    AuthenticationDialog dialog(item->text(0).toStdString(), &m_main_window);
    if (dialog.exec()) {
        std::string username = dialog.getUsername();
        std::string passwd = dialog.getPasswd();
        if (username == "" || passwd == "") {
            return;
        }

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
        widget<QPushButton *>("stop_refresh_button")->setEnabled(false);
        handleProgressState(1);
        m_main_window.getStatusBar()->clearMessage();
    }
}

void Engine::Kernel::handleConnecting(CIMClient *client,
                                      PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)");
    if (client == NULL) {
        handleProgressState(1);
        widget<QPushButton *>("stop_refresh_button")->setEnabled(false);
        return;
    }

    if (state == PowerStateValues::NoPowerSetting) {
        QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
        IPlugin *plugin = (IPlugin *) tab->currentWidget();

        if (plugin != NULL) {
            // NOTE quick enough, maybe later move to another thread
            setMac(client);

            plugin->refresh(client);
            QPushButton *button = widget<QPushButton *>("stop_refresh_button");
            button->setEnabled(true);
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
    IPlugin *plugin = (IPlugin *) tab->currentWidget();
    if (plugin == NULL) {
        return;
    }

    if (state == 100) {
        tab->setEnabled(true);
        plugin->setPluginEnabled(true);
        plugin->setRefreshed(true);
        setButtonsEnabled(true);
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
    if (!m_refreshEnabled ||
        m_main_window.getPcTreeWidget()->getTree()->selectedItems().empty()) {
        return;
    }

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();

    if (plugin == NULL) {
        return;
    }

    tab->setEnabled(false);
    setButtonsEnabled(false);
    handleProgressState(0);

    boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                              PowerStateValues::NoPowerSetting));
}

void Engine::Kernel::reloadPlugins()
{
    Logger::getInstance()->debug("Engine::Kernel::reloadPlugins()");
    for (plugin_map::iterator it = m_loaded_plugins.begin();
         it != m_loaded_plugins.end(); it++) {
        (*it).second->disconnect();
    }
    disconnect(
        m_main_window.getProviderWidget()->getTabWidget(),
        0,
        this,
        0);
    foreach (QPluginLoader * loader, m_loaders) {
        loader->unload();
        delete loader;
    }
    connect(
        m_main_window.getProviderWidget()->getTabWidget(),
        SIGNAL(currentChanged(int)),
        this,
        SLOT(setActivePlugin(int)));

    m_loaded_plugins.clear();
    m_loaders.clear();

    for (; m_main_window.getProviderWidget()->getTabWidget()->count();) {
        m_main_window.getProviderWidget()->getTabWidget()->removeTab(0);
    }

    loadPlugin();
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
        TreeWidgetItem *item = (TreeWidgetItem *)
                               m_main_window.getPcTreeWidget()->getTree()
                               ->selectedItems()[0];
        m_save_script_path += "/" + item->getName();
    }

    plugin_map::iterator it;
    for (it = m_loaded_plugins.begin(); it != m_loaded_plugins.end(); it++) {
        it->second->saveScript(m_save_script_path);
    }
}

void Engine::Kernel::selectionChanged()
{
    Logger::getInstance()->debug("Engine::Kernel::selectionChanged()");
    enableSpecialButtons(true);

    QList<QTreeWidgetItem *> list =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty()) {
        return;
    }

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setCurrentIndex(0);

    std::string id = ((TreeWidgetItem *) list[0])->getId();
    std::string title = WINDOW_TITLE;
    title += " @ " + id;
    m_main_window.setWindowTitle(title.c_str());

    title = PROVIDER_BOX_TITLE;
    title += id;
    m_main_window.getProviderWidget()->setTitle(title);

    int cnt = tab->count();
    for (int i = 0; i < cnt; i++) {
        IPlugin *plugin = (IPlugin *) tab->widget(i);
        if (plugin == NULL) {
            return;
        }

        plugin->stopRefresh();
        plugin->clear();
        plugin->setRefreshed(false);
    }

    IPlugin *plugin = (IPlugin *) tab->widget(0);
    if (plugin == NULL) {
        return;
    }

    std::string label = plugin->getLabel();
    std::transform(label.begin(), label.end(), label.begin(), ::tolower);

    if (!m_settings->value<bool, QCheckBox *>(label)) {
        return;
    }

    refresh();
}

void Engine::Kernel::setActivePlugin(int index)
{
    Logger::getInstance()->debug("Engine::Kernel::setActivePlugin(int index)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin;

    int cnt = tab->count();
    for (int i = 0; i < cnt; i++) {
        IPlugin *plugin = (IPlugin *) tab->widget(i);
        if (plugin == NULL) {
            continue;
        }

        if (i == index) {
            plugin->setActive(true);
            m_code_dialog.setText(plugin->getInstructionText(), false);
            setButtonsEnabled(true, false);
            QPushButton *button = widget<QPushButton *>("filter_button");
            if (button != NULL) {
                button->setChecked(plugin->isFilterShown());
            }
        } else {
            plugin->setActive(false);
        }
    }

    tab->setCurrentIndex(index);
    plugin = (IPlugin *) tab->currentWidget();
    if (plugin == NULL) {
        return;
    }

    std::string label = plugin->getLabel();
    std::transform(label.begin(), label.end(), label.begin(), ::tolower);

    bool enabled = m_settings->value<bool, QCheckBox *>(label);
    bool refreshed = plugin->isRefreshed();
    if (!enabled && !refreshed) {
        return;
    }       

    if (!refreshed) {
        refresh();
    } else {
        Logger::getInstance()->info(plugin->getRefreshInfo());
    }
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
    tab->setTabText(getIndexOfTab(plugin->getLabel()),
                    std::string("* " + plugin->getLabel()).c_str());
}

void Engine::Kernel::setPowerState(QAction *action)
{
    Logger::getInstance()->debug("Engine::Kernel::setPowerState(QAction *action)");
    QTreeWidgetItem *item =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string message = "";
    handleProgressState(0);
    if (action->objectName().toStdString() == "reboot_action") {
        message = "Rebooting system: ";
        m_main_window.getPcTreeWidget()->setComputerIcon(QIcon(":/reboot.png"));
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffSoft));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = "Shutting down system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffSoftGraceful));
    } else if (action->objectName().toStdString() == "force_reset_action") {
        message = "Force rebooting system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffHard));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = "Force off system: ";
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffHard));
    } else if (action->objectName().toStdString() == "wake_on_lan") {
        message = "Waking system: ";
        wakeOnLan();
    }

    Logger::getInstance()->info(message + item->text(0).toStdString());
}

void Engine::Kernel::showAboutDialog()
{
    std::string text = "LMI Command Center\n"
                       "Version: " + std::string(LMI_VERSION) + "\n\n"
                       "Authors: Martin Hatina\n"
                       "Email: mhatina@redhat.com";
    QMessageBox::about(&m_main_window, "About", text.c_str());
}

void Engine::Kernel::showCodeDialog()
{
    Logger::getInstance()->debug("Engine::Kernel::showCodeDialog()");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();
    if (plugin == NULL) {
        return;
    }
    m_code_dialog.setText(plugin->getInstructionText(), false);
    m_code_dialog.show();
}

void Engine::Kernel::showFilter()
{
    Logger::getInstance()->debug("Engine::Kernel::showFilter()");
    QPushButton *button = widget<QPushButton *>("filter_button");

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();

    if (plugin == NULL) {
        button->setChecked(false);
        return;
    }

    bool checked = plugin->showFilter(button->isChecked());
    button->setChecked(checked);
}

void Engine::Kernel::showSettings()
{
    Logger::getInstance()->debug("Engine::Kernel::showSettings()");
    m_settings->exec();
}

void Engine::Kernel::startLMIShell()
{
    Logger::getInstance()->debug("Engine::Kernel::startLMIShell()");
    std::string command =
        m_settings->value<std::string, QLineEdit *>("terminal_emulator");
    QList<QTreeWidgetItem *> list =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems();
//    if (list.empty())
    // TODO certificate
    command = " -e \" lmishell";
//    else {
//        TreeWidgetItem *item = (TreeWidgetItem*) list[0];
//        command = " -e \" lmishell"
//            + (!item->getIpv4().empty() ? item->getIpv4() : item->getIpv6()) + "\"";
//    }
    QProcess *shell = new QProcess(this);
    shell->startDetached(command.c_str());
}

void Engine::Kernel::startSsh()
{
    Logger::getInstance()->debug("Engine::Kernel::startSsh()");
    QList<QTreeWidgetItem *> list =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty()) {
        return;
    }

    TreeWidgetItem *item = (TreeWidgetItem *) list[0];
    std::string command =
        m_settings->value<std::string, QLineEdit *>("terminal_emulator");
    command += " -e \"ssh "
               + (!item->getIpv4().empty() ? item->getIpv4() : item->getIpv6()) + "\"";
    QProcess *shell = new QProcess(this);
    shell->startDetached(command.c_str());
}

void Engine::Kernel::stopRefresh()
{
    Logger::getInstance()->debug("Engine::Kernel::stopRefresh()");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();

    if (plugin == NULL) {
        return;
    }

    plugin->stopRefresh();
    Logger::getInstance()->info(plugin->getLabel() + " has been stopped");
    QPushButton *button = widget<QPushButton *>("stop_refresh_button");
    button->setEnabled(false);
}
