/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "kernel.h"
#include "cimvalue.h"
#include "logger.h"

#include <boost/thread.hpp>
#include <gnome-keyring-1/gnome-keyring.h>
#include <Pegasus/Common/Array.h>

#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;

bool isColon(int c)
{
    return (c == 0x3A);
}

Engine::Kernel::Kernel() :
    m_refreshEnabled(true),
    m_mutex(new QMutex()),
    m_bar(new ProgressBar()),
    m_last_system(NULL),
    m_settings(SettingsDialog::getInstance(&m_main_window))
{
    Logger::getInstance()->debug("Engine::Kernel::Kernel()");

    __uid_t uid = getuid();
    if (uid == 0) {
        Logger::getInstance()->error("Program does not work properly when run as root!");
        exit(1);
    }

    struct passwd *pw = getpwuid(uid);
    std::string path = pw->pw_dir;
    path += "/.config";
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST) {
        Logger::getInstance()->error("Cannot create ~/.config dir.\nError: " +
                                     std::string(strerror(errno)));
    }

    initConnections();

    m_main_window.getStatusBar()->addPermanentWidget(m_bar);
    m_main_window.getPcTreeWidget()->setTimeSec(2);
    m_bar->setMaximum(0);

    m_code_dialog.setTitle("LMIShell Code");
    createKeyring();
}

Engine::Kernel::~Kernel()
{
    Logger::getInstance()->debug("Engine::Kernel::~Kernel()");
    delete m_mutex;
    delete m_bar;
    SettingsDialog::deleteInstance();

    disconnect(
        m_main_window.getProviderWidget()->getTabWidget(),
        0,
        this,
        0);
    foreach (QPluginLoader * loader, m_loaders) {
        loader->unload();
        delete loader;
    }
    Logger::removeInstance();
}

int Engine::Kernel::getIndexOfTab(std::string name)
{
    Logger::getInstance()->debug("Engine::Kernel::getIndexOfTab(std::string name)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    for (int i = 0; i < tab->count(); i++) {
        if (tab->tabText(i).contains(name.c_str())) {
            return i;
        }
    }

    return -1;
}

std::string Engine::Kernel::getPowerStateMessage(PowerStateValues::POWER_VALUES state)
{
    QTreeWidgetItem *item =
        m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string message = "";
    switch (state) {
    case PowerStateValues::PowerCycleOffSoft:
        message = "Rebooting system: ";
        break;
    case PowerStateValues::PowerOffSoftGraceful:
        message = "Shutting down system: ";
        break;
    case PowerStateValues::PowerCycleOffHard:
        message = "Force rebooting system: ";
        break;
    case PowerStateValues::PowerOffHard:
        message = "Force off system: ";
        break;
    case PowerStateValues::WakeOnLan:
        message = "Waking system: ";
        break;
    default:
        return "Not possible!";
    }

    message += item->text(0).toStdString();
    return message;
}

void Engine::Kernel::createKeyring()
{
    Logger::getInstance()->debug("Engine::Kernel::createKeyring()");
    GnomeKeyringResult res = gnome_keyring_create_sync(OPENLMI_KEYRING_DEFAULT,
                             NULL);
    if (res != GNOME_KEYRING_RESULT_OK &&
        res != GNOME_KEYRING_RESULT_KEYRING_ALREADY_EXISTS) {
        const std::string err[] = {
            "GNOME_KEYRING_RESULT_OK",
            "GNOME_KEYRING_RESULT_DENIED",
            "GNOME_KEYRING_RESULT_NO_KEYRING_DAEMON",
            "GNOME_KEYRING_RESULT_ALREADY_UNLOCKED",
            "GNOME_KEYRING_RESULT_NO_SUCH_KEYRING",
            "GNOME_KEYRING_RESULT_BAD_ARGUMENTS",
            "GNOME_KEYRING_RESULT_IO_ERROR",
            "GNOME_KEYRING_RESULT_CANCELLED",
            "GNOME_KEYRING_RESULT_KEYRING_ALREADY_EXISTS",
            "GNOME_KEYRING_RESULT_NO_MATCH"
        };

        Logger::getInstance()->error("Cannot create " + std::string(
                                         OPENLMI_KEYRING_DEFAULT) + " keyring\nError: " + err[res]);
        exit(EXIT_FAILURE);
    }
}

void Engine::Kernel::initConnections()
{
    Logger::getInstance()->debug("Engine::Kernel::initConnections()");

    QPushButton *button = widget<QPushButton *>("refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(refresh()));
    button = widget<QPushButton *>("stop_refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(stopRefresh()));
    QToolButton *power_button = widget<QToolButton *>("power_button");
    connect(
        power_button,
        SIGNAL(triggered(QAction *)),
        this,
        SLOT(setPowerState(QAction *)));
    button = widget<QPushButton *>("delete_passwd_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(deletePasswd()));
    button = widget<QPushButton *>("show_code_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showCodeDialog()));
    button = widget<QPushButton *>("filter_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showFilter()));
    button = widget<QPushButton *>("save_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(saveScripts()));
    button = widget<QPushButton *>("save_as_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(saveAsScripts()));
    connect(
        m_main_window.getPcTreeWidget(),
        SIGNAL(removed(std::string)),
        this,
        SLOT(deletePasswd(std::string)));
    connect(
        m_main_window.getProviderWidget()->getTabWidget(),
        SIGNAL(currentChanged(int)),
        this,
        SLOT(setActivePlugin(int)));
    connect(
        m_main_window.getPcTreeWidget(),
        SIGNAL(selectionChanged()),
        this,
        SLOT(selectionChanged()));
    qRegisterMetaType<PowerStateValues::POWER_VALUES>("PowerStateValues::POWER_VALUES");
    connect(
        this,
        SIGNAL(doneConnecting(CIMClient *, PowerStateValues::POWER_VALUES)),
        this,
        SLOT(handleConnecting(CIMClient *, PowerStateValues::POWER_VALUES)));
    connect(
        this,
        SIGNAL(authenticate(PowerStateValues::POWER_VALUES)),
        this,
        SLOT(handleAuthentication(PowerStateValues::POWER_VALUES)));
    connect(
        m_main_window.getResetPasswdStorageAction(),
        SIGNAL(triggered()),
        this,
        SLOT(resetKeyring()));
    QAction *action = widget<QAction *>("action_reload_plugins");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(reloadPlugins()));
    action = widget<QAction *>("action_options");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showSettings()));
    action = widget<QAction *>("action_about");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showAboutDialog()));
    action = widget<QAction *>("action_report_bug");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(reportBug()));
    action = widget<QAction *>("delete_passwd_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(deletePasswd()));
    action = widget<QAction *>("refresh_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(selectionChanged()));
    action = widget<QAction *>("start_ssh_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(startSsh()));
    action = widget<QAction *>("start_lmishell_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(startLMIShell()));
    connect(
        m_main_window.getPcTreeWidget(),
        SIGNAL(refreshProgress(int, std::string)),
        this,
        SLOT(handleProgressState(int, std::string)));
}

void Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)
{
    Logger::getInstance()->debug("Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin *) tab->currentWidget();
    if (plugin == NULL) {
        return;
    }
    bool refreshed = plugin->isRefreshed();

    ((QPushButton *) widget<QPushButton *>("apply_button"))->setEnabled(
        state & refreshed);
    ((QPushButton *) widget<QPushButton *>("cancel_button"))->setEnabled(
        state & refreshed);
    ((QPushButton *) widget<QPushButton *>("save_button"))->setEnabled(
        state & refreshed);
    ((QPushButton *) widget<QPushButton *>("save_as_button"))->setEnabled(
        state & refreshed);
    ((QPushButton *) widget<QPushButton *>("stop_refresh_button"))->setEnabled(
        !state);
    if (refresh_button) {
        enableSpecialButtons(state);
    }
}

void Engine::Kernel::setMac(CIMClient *client)
{
    Logger::getInstance()->debug("Engine::Kernel::setMac(CIMClient *client)");
    QTreeWidget *tree = m_main_window.getPcTreeWidget()->getTree();
    QList<QTreeWidgetItem *> list = tree->findItems(client->hostname().c_str(),
                                    Qt::MatchExactly | Qt::MatchRecursive);
    if (list.empty()) {
        return;
    }

    TreeWidgetItem *item = (TreeWidgetItem *) list[0];

    if (!item->getMac().empty()) {
        return;
    }

    try {
        Pegasus::Array<Pegasus::CIMObject> lan =
            client->execQuery(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::String("WQL"),
                Pegasus::String("SELECT * FROM LMI_LanEndpoint WHERE OperatingStatus = 16"));

        if (lan.size() == 0) {
            return;
        }
        Pegasus::CIMObject endpoint = lan[0];

        std::string mac = CIMValue::get_property_value(Pegasus::CIMInstance(endpoint),
                          "MACAddress");
        item->setMac(mac);
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()), false);
    }
}

void Engine::Kernel::setPowerState(CIMClient *client,
                                   PowerStateValues::POWER_VALUES power_state)
{
    Logger::getInstance()->debug("Engine::Kernel::setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state)");

    Pegasus::CIMObjectPath power_inst_name;
    try {
        power_inst_name = client->enumerateInstanceNames(
                              Pegasus::CIMNamespaceName("root/cimv2"),
                              Pegasus::CIMName("LMI_PowerManagementService")
                          )[0];
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }

    Pegasus::Array<Pegasus::CIMParamValue> in_param;
    Pegasus::Array<Pegasus::CIMParamValue> out_param;

    in_param.append(Pegasus::CIMParamValue(
                        Pegasus::String("PowerState"),
                        Pegasus::CIMValue(
                            Pegasus::Uint16(power_state)
                        )
                    ));
    try {
        client->invokeMethod(
            Pegasus::CIMNamespaceName("root/cimv2"),
            power_inst_name,
            Pegasus::CIMName("RequestPowerStateChange"),
            in_param,
            out_param
        );
        m_connections.erase(m_connections.find(client->hostname()));
        client->disconnect();
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}

void Engine::Kernel::wakeOnLan()
{
    Logger::getInstance()->debug("Engine::Kernel::wakeOnLan()");
    // packet creating
    unsigned char to_send[102];
    unsigned char mac[6];

    TreeWidgetItem *item = ((TreeWidgetItem *)
                            m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]);
    std::string mac_str = item->getMac();
    if (mac_str.empty()) {
        Logger::getInstance()->error("Unknown MAC address");
        return;
    }

    // first 6 bytes of 255
    for (int i = 0; i < 6; i++) {
        to_send[i] = 0xFF;
    }

    mac_str.erase(remove_if(mac_str.begin(), mac_str.end(), isColon),
                  mac_str.end());
    // store mac address
    for (int i = 0; i < 6; i++) {
        char *p;
        int tmp = strtol(mac_str.substr(i * 2, 2).c_str(), & p, 16 );
        mac[i] = tmp;
    }

    // append it 16 times to packet
    for (int i = 1; i <= 16; i++) {
        memcpy(&to_send[i * 6], &mac, 6 * sizeof(unsigned char));
    }

    // ********************************************************************
    // sending packet

    int udp_socket;
    struct sockaddr_in udp_client, udp_server;
    int broadcast = 1;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // need to set this to be able to broadcast **/
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast,
                   sizeof broadcast) == -1) {
        Logger::getInstance()->error(strerror(errno));
        return;
    }
    udp_client.sin_family = AF_INET;
    udp_client.sin_addr.s_addr = INADDR_ANY;
    udp_client.sin_port = 0;

    bind(udp_socket, (struct sockaddr *)&udp_client, sizeof(udp_client));

    // set server end point (the broadcast addres)
    udp_server.sin_family = AF_INET;

    std::string ip = item->getIpv4();
    ip = ip.substr(0, ip.rfind(".") + 1);
    ip += "255";
    udp_server.sin_addr.s_addr = inet_addr(ip.c_str());
    udp_server.sin_port = htons(9);

    // send the packet
    sendto(udp_socket, &to_send, sizeof(unsigned char) * 102, 0,
           (struct sockaddr *)&udp_server, sizeof(udp_server));

    handleProgressState(Engine::REFRESHED);
}

void Engine::Kernel::deletePlugins()
{
    Logger::getInstance()->debug("Engine::Kernel::deletePlugins()");
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
}

void Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->debug("Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)");
    TreeWidgetItem *item = (TreeWidgetItem *)
                           m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->getId();
    Logger::getInstance()->info("Connecting to " + item->text(0).toStdString());

    switch (getSilentConnection(ip, false)) {
    case 0:
        emit doneConnecting(m_connections[ip], state);
        break;
    case 1:
        emit authenticate(state);
        break;
    case -1:
        // error already displayed
        handleProgressState(Engine::ERROR);
        break;
    }
}

void Engine::Kernel::loadPlugin()
{
    Logger::getInstance()->debug("Engine::Kernel::loadPlugin()");
    QDir plugins_dir(qApp->applicationDirPath());
    plugins_dir.cd(STR(PLUGIN_PATH));

    QPluginLoader *plugin_loader  = NULL;
    foreach (QString file_name, plugins_dir.entryList(QDir::Files)) {
        plugin_loader = new QPluginLoader(plugins_dir.absoluteFilePath(file_name));
        IPlugin *plugin = qobject_cast<IPlugin *>(plugin_loader->instance());
        m_loaders.push_back(plugin_loader);
        if (plugin && plugin != NULL) {
            if (m_loaded_plugins.find(file_name.toStdString()) == m_loaded_plugins.end()) {
                Logger::getInstance()->debug("Loaded: " + plugin->getLabel(), true);
                m_loaded_plugins[file_name.toStdString()] = plugin;

                plugin->setMutex(m_mutex);
                plugin->connectButtons(m_main_window.getToolbar());
                connect(plugin, SIGNAL(deletePasswd()), this,
                        SLOT(deletePasswd()));
                connect(plugin, SIGNAL(unsavedChanges(IPlugin *)), this,
                        SLOT(setPluginUnsavedChanges(IPlugin *)));
                connect(plugin, SIGNAL(noChanges(IPlugin *)), this,
                        SLOT(setPluginNoChanges(IPlugin *)));
                connect(plugin, SIGNAL(refreshProgress(int, IPlugin *)), this,
                        SLOT(handleProgressState(int, IPlugin *)));
                connect(plugin, SIGNAL(refreshProgress(int, std::string, IPlugin *)), this,
                        SLOT(handleProgressState(int, std::string, IPlugin *)));
                connect(plugin, SIGNAL(newInstructionText(std::string)), this,
                        SLOT(handleInstructionText(std::string)));

                if (plugin->getLabel() == "Overview") {
                    m_main_window.getProviderWidget()->getTabWidget()->insertTab(0, plugin,
                            plugin->getLabel().c_str());
                } else {
                    m_main_window.getProviderWidget()->getTabWidget()->addTab(plugin,
                            plugin->getLabel().c_str());
                }
            }
        } else {
            Logger::getInstance()->critical(plugin_loader->errorString().toStdString(), false);
        }
    }
    setActivePlugin(0);
}

void Engine::Kernel::showMainWindow()
{
    Logger::getInstance()->debug("Engine::Kernel::showMainWindow()");
    loadPlugin();
    m_main_window.show();
}
