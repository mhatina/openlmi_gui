#include "kernel.h"
#include "cimvalue.h"
#include "dialogs/settingsdialog.h"

#include <QAction>
#include <QCheckBox>
#include <QDesktopServices>
#include <QLineEdit>
#include <QProcess>
#include <QStatusBar>
#include <QString>
#include <QToolButton>
#include <QUrl>

#define BUG_REPORT_URL "https://github.com/mhatina/openlmi_gui/issues"

static int transformLabel(int c)
{
    if (c == ' ') {
        return '_';
    } else {
        return tolower(c);
    }
}

int Engine::Kernel::getSilentConnection(String ip, bool silent)
{
    Logger::getInstance()->debug("Engine::Kernel::getSilentConnection(String ip)");
    if (m_connections.find(ip) == m_connections.end()) {
        CIMClient *client = NULL;
        String username, passwd;
        if (!m_storage.unlock()) {
            return PASSWD_ERR;
        }

        switch (m_storage.getUserData(ip, username, passwd)) {
        case PASSWD_NO_MATCH:
            return PASSWD_NO_MATCH;
        case PASSWD_OK:
            client = new CIMClient();
            try {
                bool verify = false;
                String cert;

                try {
                    verify = m_settings->value<bool, QCheckBox *>("use_certificate_checkbox");
                    cert = m_settings->value<String, QLineEdit *>("certificate");

                    client->setVerifyCertificate(verify);
                    client->connect(ip, 5989, true, username, passwd, cert);
                } catch (Pegasus::Exception &ex) {
                    Logger::getInstance()->info(CIMValue::to_string(ex.getMessage()) +
                                                " Trying another port.");
                    client->setVerifyCertificate(verify);
                    client->connect(ip, 5988, false, username, passwd, cert);
                }
                m_connections[ip] = client;
                return PASSWD_OK;
            } catch (Pegasus::Exception &ex) {
                if (!silent) {
                    Logger::getInstance()->error(CIMValue::to_string(ex.getMessage()));
                }
                return PASSWD_ERR;
            }
            break;
        case PASSWD_ERR:
            Logger::getInstance()->error("Cannot get username and password: " + m_storage.errString());
        default:
            return PASSWD_ERR;
        }
    }

    return PASSWD_OK;
}

void Engine::Kernel::changeButtonConnection(bool control)
{
    changeRefreshConnection(!control);
    QPushButton *save_button = widget<QPushButton *>("save_button");
    if (control) {
        connect(
            save_button,
            SIGNAL(clicked()),
            this,
            SLOT(saveAsScripts()));
        disconnect(
            save_button,
            SIGNAL(clicked()),
            this,
            SLOT(saveScripts()));
        save_button->setIcon(QIcon(":/save-as.png"));
    } else {
        connect(
            save_button,
            SIGNAL(clicked()),
            this,
            SLOT(saveScripts()));
        disconnect(
            save_button,
            SIGNAL(clicked()),
            this,
            SLOT(saveAsScripts()));
        save_button->setIcon(QIcon(":/save.png"));
    }
}

void Engine::Kernel::deletePasswd()
{
    Logger::getInstance()->debug("Engine::Kernel::deletePasswd()");
    TreeWidgetItem *item = (TreeWidgetItem *) m_main_window.getPcTreeWidget()->
                           getTree()->selectedItems()[0];
    deletePasswd(item->getId());
}

void Engine::Kernel::deletePasswd(String id)
{
    Logger::getInstance()->debug("Engine::Kernel::deletePasswd(String id)");

    CIMClient *client = NULL;
    if (!m_connections.empty() && m_connections.find(id) != m_connections.end()) {
        client = (*m_connections.find(id)).second;
        if (client && client->isConnected()) {
            client->disconnect();
            m_connections.erase(m_connections.find(id));
        }
    }

    if (!m_storage.deletePassword(id)) {
        Logger::getInstance()->info("Cannot delete password: " + m_storage.errString());
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

    String ip = item->getId();
    AuthenticationDialog dialog(item->text(0), &m_main_window);
    if (dialog.exec()) {
        String username = dialog.getUsername();
        String passwd = dialog.getPasswd();

        if (username == "" || passwd == "") {
            handleProgressState(Engine::ERROR);
            Logger::getInstance()->error("Username/password cannot be empty");
            handleAuthentication(state);
            return;
        }

        if (!m_storage.setUserData(ip, username, passwd)) {
            handleProgressState(Engine::ERROR);
            Logger::getInstance()->error("Cannot store password: " + m_storage.errString());
            return;
        }

        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, state));
    } else {
        handleProgressState(Engine::ERROR);
        m_main_window.getStatusBar()->clearMessage();
    }
}

void Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)
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
        }
    } else {
        setPowerState(client, state);
        String message = getPowerStateMessage(state);
        Logger::getInstance()->info(message);
        handleProgressState(Engine::REFRESHED, message);
    }
}

void Engine::Kernel::handleInstructionText(String text)
{
    Logger::getInstance()->debug("Engine::Kernel::handleInstructionText(String text)");
    m_code_dialog.setText(text);
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

void Engine::Kernel::handleProgressState(int state, String process, IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::handleProgressState(int state, String process, IPlugin *plugin)");
    switch (state) {
    case REFRESHED:
        m_bar->hide(process);
    case ALMOST_REFRESHED:
        setButtonsEnabled(true);
        break;
    case NOT_REFRESHED:
        m_bar->show(process);
        break;
    case STOP_REFRESH:
    case ERROR:
        enableSpecialButtons(true);
        m_bar->hide(process);
        break;
    default:
        Logger::getInstance()->critical("Unknown refresh state!");
        break;
    }

    if (plugin != NULL) {
        switch (state) {
        case REFRESHED:
            if (m_bar->empty()) {
                changeRefreshConnection(true);
            }
        case ALMOST_REFRESHED:
            plugin->setPluginEnabled(true);
            plugin->setRefreshed(true);
            break;
        case STOP_REFRESH:
            if (m_bar->empty()) {
                changeRefreshConnection(true);
            }
            break;
        case NOT_REFRESHED:
            changeRefreshConnection(false);
            plugin->setPluginEnabled(false);
            break;
        case ERROR:
            changeRefreshConnection(true);
            plugin->setPluginEnabled(false);
            break;
        default:
            Logger::getInstance()->critical("Unknown refresh state!");
            break;
        }
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

    if (plugin == NULL || plugin->isRefreshing()) {
        return;
    }

    setButtonsEnabled(false, false);
    handleProgressState(Engine::NOT_REFRESHED);

    boost::thread(
        boost::bind(
            &Engine::Kernel::getConnection,
            this,
            PowerStateValues::NoPowerSetting
        )
    );
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
        Logger::getInstance()->error("Cannot open : " + url);
    }
}

void Engine::Kernel::resetPasswdStorage()
{
    Logger::getInstance()->debug("Engine::Kernel::resetPasswdStorage()");
    if (!m_storage.resetStorage()) {
        Logger::getInstance()->error("Cannot reset password storage: " + m_storage.errString());
        return;
    }
}

void Engine::Kernel::saveAsScripts()
{
    Logger::getInstance()->debug("Engine::Kernel::saveAsScripts()");
    m_save_script_path = "";
    changeButtonConnection(false);
    saveScripts();
}

void Engine::Kernel::saveScripts()
{
    Logger::getInstance()->debug("Engine::Kernel::saveScripts()");
    if (m_save_script_path.empty()) {
        QFileDialog dialog(&m_main_window);
        dialog.setFileMode(QFileDialog::AnyFile);
        m_save_script_path = dialog.getExistingDirectory();
        TreeWidgetItem *item = (TreeWidgetItem *)
                               m_main_window.getPcTreeWidget()->getTree()
                               ->selectedItems()[0];
        m_save_script_path += "_" + item->getName();
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

        String id = ((TreeWidgetItem *) list[0])->getId();
        String title = WINDOW_TITLE;
        title += " @ ";
        title += id;
        m_main_window.setWindowTitle(title);

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

    String label = plugin->getLabel();
    std::transform(label.begin(), label.end(), label.begin(), transformLabel);

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
            m_code_dialog.setText(plugin->getInstructionText());
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

    String label = plugin->getLabel();
    std::transform(label.begin(), label.end(), label.begin(), transformLabel);

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
    tab->setTabText(getIndexOfTab(plugin->getLabel()), plugin->getLabel());
}

void Engine::Kernel::setPluginUnsavedChanges(IPlugin *plugin)
{
    Logger::getInstance()->debug("Engine::Kernel::setPluginUnsavedChanges(IPlugin *plugin)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setTabText(getIndexOfTab(plugin->getLabel()),
                    String("* " + plugin->getLabel()));
}

void Engine::Kernel::setPowerState(QAction *action)
{
    Logger::getInstance()->debug("Engine::Kernel::setPowerState(QAction *action)");
    String message = "";
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


    if (action->objectName() == "reboot_action") {
        message = getPowerStateMessage(PowerStateValues::PowerCycleOffSoft);
        m_main_window.getPcTreeWidget()->setComputerIcon(QIcon(":/reboot.png"));
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffSoft));
    } else if (action->objectName() == "shutdown_action") {
        message = getPowerStateMessage(PowerStateValues::PowerOffSoftGraceful);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffSoftGraceful));
    } else if (action->objectName() == "force_reset_action") {
        message = getPowerStateMessage(PowerStateValues::PowerCycleOffHard);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerCycleOffHard));
    } else if (action->objectName() == "shutdown_action") {
        message = getPowerStateMessage(PowerStateValues::PowerOffHard);
        boost::thread(boost::bind(&Engine::Kernel::getConnection, this,
                                  PowerStateValues::PowerOffHard));
    } else if (action->objectName() == "wake_on_lan") {
        message = getPowerStateMessage(PowerStateValues::WakeOnLan);
        wakeOnLan();
    }

    handleProgressState(Engine::NOT_REFRESHED, message);
}

void Engine::Kernel::showAboutDialog()
{
    QString text = "LMI Command Center\n"
                   "Version: ";
    text.append(LMICC_VERSION);
    text.append("\n\nAuthors: Martin Hatina\n"
                "Email: mhatina@redhat.com");

    QMessageBox box;
    box.setObjectName("about_dialog");
    box.about(&m_main_window, "About", text);
}

void Engine::Kernel::showCodeDialog()
{
    Logger::getInstance()->debug("Engine::Kernel::showCodeDialog()");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();
    if (plugin == NULL) {
        return;
    }
    m_code_dialog.setText(plugin->getInstructionText());
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

void Engine::Kernel::showHelp()
{
    QString url = STR(DOC_PATH);
    url.append("/help/index.html");

    if (!QDesktopServices::openUrl(QUrl(url))) {
        Logger::getInstance()->error("Cannot open : " + url);
    }
}

void Engine::Kernel::showSettings()
{
    Logger::getInstance()->debug("Engine::Kernel::showSettings()");
    m_settings->exec();
}

void Engine::Kernel::startLMIShell()
{
    Logger::getInstance()->debug("Engine::Kernel::startLMIShell()");
    String command =
        m_settings->value<String, QLineEdit *>("terminal_emulator");
    QList<QTreeWidgetItem *> list =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    if (list.empty()) {
        return;
    }


    TreeWidgetItem *item = (TreeWidgetItem *) list[0];
    String connect = "'c = connect(\"" + item->getId() + "\")'";
    // TODO certificate
    command += " -e \"lmishell -c " + connect + "\"";

    QProcess shell(this);
    shell.startDetached(command);
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
    String command =
        m_settings->value<String, QLineEdit *>("terminal_emulator");
    command += " -e \"ssh "
               + (!item->getIpv4().empty() ? item->getIpv4() : item->getIpv6()) + "\"";
    QProcess shell(this);
    shell.startDetached(command);
}

void Engine::Kernel::stopRefresh()
{
    Logger::getInstance()->debug("Engine::Kernel::stopRefresh()");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin;
    for (int i = 0; i < tab->count(); i++) {
        plugin = (IPlugin *) tab->widget(i);
        if (plugin == NULL) {
            continue;
        }
        plugin->stopRefresh();
    }

    Logger::getInstance()->info("All plugins have been stopped");
}
