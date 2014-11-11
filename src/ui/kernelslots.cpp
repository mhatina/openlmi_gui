#include "kernel.h"
#include "cimvalue.h"
#include "settingsdialog.h"

#include <gnome-keyring-1/gnome-keyring.h>
#include <QCheckBox>
#include <QDesktopServices>
#include <QLineEdit>
#include <QProcess>
#include <QStatusBar>
#include <QString>
#include <QToolButton>
#include <QUrl>

#define BUG_REPORT_URL "https://github.com/mhatina/openlmi_gui/issues"

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
            std::string err = gnome_keyring_result_to_message(res);
            Logger::getInstance()->error("Cannot get username and password: " + err);
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
                Logger::getInstance()->info(CIMValue::to_std_string(ex.getMessage()) +
                                            " Trying another port.");
                client->setVerifyCertificate(verify);
                client->connect(ip, 5988, false, username, passwd, cert);
            }
            m_connections[ip] = client;
            return 0;
        } catch (Pegasus::Exception &ex) {
            if (!silent) {
                Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
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

    CIMClient *client = NULL;
    if (!m_connections.empty())
        client = (*m_connections.find(id)).second;
    if (client && client->isConnected()) {
        client->disconnect();
        m_connections.erase(m_connections.find(id));
    }

    GnomeKeyringResult res = gnome_keyring_delete_password_sync(
                                 GNOME_KEYRING_NETWORK_PASSWORD,
                                 "server", id.c_str(),
                                 NULL
                             );

    if (res != GNOME_KEYRING_RESULT_OK) {
        std::string err = gnome_keyring_result_to_message(res);
        Logger::getInstance()->info("Cannot delete password: " + err);
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
    QList<QTreeWidgetItem *> list = m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty()) {
        handleProgressState(Engine::ERROR);
        Logger::getInstance()->error("No system to authenticate");
        return;
    }
    TreeWidgetItem *item = (TreeWidgetItem *) list[0];

    std::string ip = item->getId();
    AuthenticationDialog dialog(item->text(0).toStdString(), &m_main_window);
    if (dialog.exec()) {
        std::string username = dialog.getUsername();
        std::string passwd = dialog.getPasswd();

        if (username == "" || passwd == "") {
            handleProgressState(Engine::ERROR);
            Logger::getInstance()->error("Username/password cannot be empty");
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
            handleProgressState(Engine::ERROR);
            std::string err = gnome_keyring_result_to_message(res);
            Logger::getInstance()->error("Cannot store password: " + err);
            return;
        }

        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, state));
    } else {
        handleProgressState(Engine::ERROR);
        m_main_window.getStatusBar()->clearMessage();
    }
}

void Engine::Kernel::handleConnecting(CIMClient *client,
                                      PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)");
    if (client == NULL) {
        if (PowerStateValues::NoPowerSetting) {
            handleProgressState(Engine::ERROR);
        } else {
            handleProgressState(Engine::ERROR, getPowerStateMessage(state));
        }
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
        std::string message = getPowerStateMessage(state);
        Logger::getInstance()->info(message);
        handleProgressState(Engine::REFRESHED, message);
    }
}

void Engine::Kernel::handleInstructionText(std::string text)
{
    Logger::getInstance()->debug("Engine::Kernel::handleInstructionText(std::string text)");
    m_code_dialog.setText(text, false);
}

void Engine::Kernel::handleProgressState(int state, IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::handleProgressState(int state, IPlugin *plugin)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    if (plugin == NULL && (plugin = (IPlugin *) tab->currentWidget()) == NULL) {
        return;
    }
    handleProgressState(state, "Refreshing: " + plugin->getLabel(), plugin);
}

void Engine::Kernel::handleProgressState(int state, std::string process, IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::handleProgressState(int state, std::string process, IPlugin *plugin)");
    if (plugin != NULL) {
        switch (state) {
        case REFRESHED:
        case ALMOST_REFRESHED:
            plugin->setPluginEnabled(true);
            plugin->setRefreshed(true);
            break;
        case NOT_REFRESHED:
            plugin->setPluginEnabled(false);
            break;
        case ERROR:
            plugin->setPluginEnabled(false);
            widget<QPushButton *>("stop_refresh_button")->setEnabled(false);
            break;
        default:
            Logger::getInstance()->critical("Unknown refresh state!");
            break;
        }
    }

    switch (state) {
    case REFRESHED:
        m_bar->hide(process);
    case ALMOST_REFRESHED:
        setButtonsEnabled(true);
        break;
    case NOT_REFRESHED:
        m_bar->show(process);
        break;
    case ERROR:
        enableSpecialButtons(true);
        m_bar->hide(process);
        break;
    default:
        Logger::getInstance()->critical("Unknown refresh state!");
        break;
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

    setButtonsEnabled(false, false);
    handleProgressState(Engine::NOT_REFRESHED);

    boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                              PowerStateValues::NoPowerSetting));
}

void Engine::Kernel::reloadPlugins()
{
    Logger::getInstance()->debug("Engine::Kernel::reloadPlugins()");
    deletePlugins();
    loadPlugin();
}

void Engine::Kernel::reportBug()
{
    QString url = BUG_REPORT_URL;
    if (!QDesktopServices::openUrl(QUrl(url))) {
        Logger::getInstance()->error("Cannot open : " + url.toStdString());
    }
}

void Engine::Kernel::resetKeyring()
{
    Logger::getInstance()->debug("Engine::Kernel::resetKeyring()");
    GnomeKeyringResult res = gnome_keyring_delete_sync(OPENLMI_KEYRING_DEFAULT);
    if (res != GNOME_KEYRING_RESULT_OK) {
        std::string err = gnome_keyring_result_to_message(res);
        Logger::getInstance()->error("Cannot reset keyring: " + err);
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
    IPlugin *plugin = (IPlugin *) tab->currentWidget();

    if (list[0] == m_last_system) {
        if (plugin->isRefreshed()) {
            return;
        }
    } else {
        m_last_system = list[0];
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
            plugin = (IPlugin *) tab->widget(i);
            if (plugin == NULL) {
                return;
            }

            plugin->stopRefresh();
            plugin->clear();
            plugin->setRefreshed(false);
        }
    }

    plugin = (IPlugin *) tab->currentWidget();
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
    std::string message = "";
    QMessageBox box;
    box.setObjectName("confirm_action_dialog");
    switch (box.information(
                &m_main_window,
                "Confirm action",
                "Do you really want to perform this action?",
                QMessageBox::Ok,
                QMessageBox::Cancel)) {
    case QMessageBox::Ok:
        break;
    case QMessageBox::Cancel:
    default:
        return;
    }


    if (action->objectName().toStdString() == "reboot_action") {
        message = getPowerStateMessage(PowerStateValues::PowerCycleOffSoft);
        m_main_window.getPcTreeWidget()->setComputerIcon(QIcon(":/reboot.png"));
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffSoft));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = getPowerStateMessage(PowerStateValues::PowerOffSoftGraceful);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffSoftGraceful));
    } else if (action->objectName().toStdString() == "force_reset_action") {
        message = getPowerStateMessage(PowerStateValues::PowerCycleOffHard);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffHard));
    } else if (action->objectName().toStdString() == "shutdown_action") {
        message = getPowerStateMessage(PowerStateValues::PowerOffHard);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffHard));
    } else if (action->objectName().toStdString() == "wake_on_lan") {
        message = getPowerStateMessage(PowerStateValues::WakeOnLan);
        wakeOnLan();
    }

    handleProgressState(Engine::NOT_REFRESHED, message);
}

void Engine::Kernel::showAboutDialog()
{
    std::string text = "LMI Command Center\n"
                       "Version: " + std::string(LMI_VERSION) + "\n\n"
                       "Authors: Martin Hatina\n"
                       "Email: mhatina@redhat.com";
    QMessageBox box;
    box.setObjectName("about_dialog");
    box.about(&m_main_window, "About", text.c_str());
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
    if (list.empty()) {
        return;
    }

    // TODO certificate
    command += " -e \"lmishell\"";

    TreeWidgetItem *item = (TreeWidgetItem *) list[0];

    QProcess shell(this);
    std::string connect = "c = connect(\"" + item->getId() + "\")\n";
    shell.startDetached(command.c_str());

    shell.waitForStarted();

    // NOTE not working
    qint64 i = shell.write(connect.c_str());
    shell.closeWriteChannel();
    std::cerr << i << "\n";
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
    QProcess shell(this);
    shell.startDetached(command.c_str());
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
