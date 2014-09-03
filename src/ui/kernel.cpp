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

#include "eventlog.h"
#include "kernel.h"
#include "lmiwbem_value.h"
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

#define STR(X)  __STRING(X)

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;

bool isColon(int c)
{
    return (c == 0x3A);
}

Engine::Kernel::Kernel() :    
    m_refreshEnabled(true),
    m_event_log(new EventLog()),
    m_mutex(new QMutex()),
    m_bar(new QProgressBar()),
    settings(new SettingsDialog(&m_main_window))
{
    Logger::getInstance()->debug("Engine::Kernel::Kernel()");

    __uid_t uid = getuid();
    if (uid == 0) {
        Logger::getInstance()->error("Program does not work properly when run as root!");
        exit(1);
    }

    struct passwd *pw = getpwuid(uid);
    std::string path = pw->pw_dir;
    path += "/.openlmi";
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST) {
        Logger::getInstance()->error("Cannot create ~/.openlmi dir.\nError: " + std::string(strerror(errno)));
    }

    initConnections();

    m_main_window.getStatusBar()->addPermanentWidget(m_bar);
    m_bar->setMaximumWidth(150);
    m_bar->hide();

    m_main_window.getPcTreeWidget()->setTimeSec(1.5);

    m_code_dialog.setTitle("LMIShell Code");
    createKeyring();        

    m_event_log->setConnectionStorage(&m_connections);
    m_event_log->setPCTree(m_main_window.getPcTreeWidget()->getTree());
//    m_event_log->start();
}

Engine::Kernel::~Kernel()
{
    Logger::getInstance()->debug("Engine::Kernel::~Kernel()");    
//    m_event_log->end();
    delete m_event_log;
    delete m_mutex;
    delete m_bar;
    delete settings;

    disconnect(
        m_main_window.getProviderWidget()->getTabWidget(),
        0,
        this,
        0);
    foreach (QPluginLoader *loader, m_loaders) {
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
       if (tab->tabText(i).contains(name.c_str()))
           return i;
    }

    return -1;
}

void Engine::Kernel::createKeyring()
{
    Logger::getInstance()->debug("Engine::Kernel::createKeyring()");
    GnomeKeyringResult res = gnome_keyring_create_sync(OPENLMI_KEYRING_DEFAULT, NULL);
    if (res != GNOME_KEYRING_RESULT_OK && res != GNOME_KEYRING_RESULT_KEYRING_ALREADY_EXISTS) {
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

        Logger::getInstance()->error("Cannot create " + std::string(OPENLMI_KEYRING_DEFAULT) + " keyring\nError: " + err[res]);
        exit(EXIT_FAILURE);
    }
}

void Engine::Kernel::initConnections()
{
    Logger::getInstance()->debug("Engine::Kernel::initConnections()");

    QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(refresh()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("stop_refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(stopRefresh()));
    QToolButton *power_button = m_main_window.getToolbar()->findChild<QToolButton*>("power_button");
    connect(
        power_button,
        SIGNAL(triggered(QAction*)),
        this,
        SLOT(setPowerState(QAction*)));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(deletePasswd()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("show_code_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showCodeDialog()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showFilter()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("save_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(saveScripts()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("save_as_button");
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
        m_main_window.getPcTreeWidget()->getTree(),
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(selectionChanged()));
    qRegisterMetaType<PowerStateValues::POWER_VALUES>("PowerStateValues::POWER_VALUES");
    connect(
        this,
        SIGNAL(doneConnecting(CIMClient*,PowerStateValues::POWER_VALUES)),
        this,
        SLOT(handleConnecting(CIMClient*,PowerStateValues::POWER_VALUES)));
    connect(
        this,
        SIGNAL(error(std::string)),
        this,
        SLOT(handleError(std::string)));
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
    connect(
        m_event_log,
        SIGNAL(silentConnection(std::string)),
        this,
        SLOT(emitSilentConnection(std::string)));
    QAction *action = m_main_window.findChild<QAction*>("action_start_LMIShell");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(startLMIShell()));
    action = m_main_window.findChild<QAction*>("action_start_ssh");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(startSsh()));
    action = m_main_window.findChild<QAction*>("action_reload_plugins");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(reloadPlugins()));
    action = m_main_window.findChild<QAction*>("action_options");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showSettings()));
}

void Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)
{
    Logger::getInstance()->debug("Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)");
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    if (plugin == NULL)
        return;
    bool refreshed = plugin->isRefreshed();

    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("apply_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("cancel_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_as_button"))->setEnabled(state & refreshed);
    if (refresh_button) {
        enableSpecialButtons(state);
    }    
}

void Engine::Kernel::setMac(CIMClient *client)
{
    QTreeWidget *tree = m_main_window.getPcTreeWidget()->getTree();
    QList<QTreeWidgetItem*> list = tree->findItems(client->hostname().c_str(), Qt::MatchExactly | Qt::MatchRecursive);
    if (list.empty())
        return;

    TreeWidgetItem *item = (TreeWidgetItem*) list[0];

    if (!item->getMac().empty())
        return;

    try {
        Pegasus::Array<Pegasus::CIMObject> lan =
                client->execQuery(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::String("WQL"),
                    Pegasus::String("SELECT * FROM LMI_LanEndpoint WHERE OperatingStatus = 16"));

        if (lan.size() == 0)
            return;
        Pegasus::CIMObject endpoint = lan[0];

        std::string mac = CIMValue::get_property_value(Pegasus::CIMInstance(endpoint), "MACAddress");
        item->setMac(mac);

        // temporary
        Pegasus::Array<Pegasus::CIMObject> ip =
                client->associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    endpoint.getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_IPProtocolEndpoint"));

        for (unsigned int i = 0; i < ip.size(); i++) {
            std::string ipv6 = CIMValue::get_property_value(Pegasus::CIMInstance(ip[i]), "IPv6Address");
            if (!ipv6.empty()) {
                item->setIpv6(ipv6);
                break;
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()), false);
    }
}

void Engine::Kernel::setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state)
{
    Logger::getInstance()->debug("Engine::Kernel::setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state)");

    Pegasus::CIMObjectPath power_inst_name;
    try {
        power_inst_name = client->enumerateInstanceNames(
            Pegasus::CIMNamespaceName("root/cimv2"),
            Pegasus::CIMName("LMI_PowerManagementService")
            )[0];
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
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
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }
}

void Engine::Kernel::wakeOnLan()
{
    // packet creating
    unsigned char to_send[102];
    unsigned char mac[6];

    TreeWidgetItem *item = ((TreeWidgetItem*) m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]);
    std::string mac_str = item->getMac();
    if (mac_str.empty()) {
        Logger::getInstance()->error("Unknown MAC address");
        return;
    }

    // first 6 bytes of 255
    for(int i = 0; i < 6; i++) {
        to_send[i] = 0xFF;
    }

    mac_str.erase(remove_if(mac_str.begin(), mac_str.end(), isColon), mac_str.end());
    // store mac address
    for (int i = 0; i < 6; i++) {
        char *p;
        int tmp = strtol(mac_str.substr(i * 2, 2).c_str(), & p, 16 );
        mac[i] = tmp;
    }

    // append it 16 times to packet
    for(int i = 1; i <= 16; i++) {
        memcpy(&to_send[i * 6], &mac, 6 * sizeof(unsigned char));
    }

    // ********************************************************************
    // sending packet

    int udp_socket;
    struct sockaddr_in udp_client, udp_server;
    int broadcast = 1;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // need to set this to be able to broadcast **/
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        Logger::getInstance()->error(strerror(errno));
        return;
    }
    udp_client.sin_family = AF_INET;
    udp_client.sin_addr.s_addr = INADDR_ANY;
    udp_client.sin_port = 0;

    bind(udp_socket, (struct sockaddr*)&udp_client, sizeof(udp_client));

    // set server end point (the broadcast addres)
    udp_server.sin_family = AF_INET;

    std::string ip = item->getIpv4();
    ip = ip.substr(0, ip.rfind(".") + 1);
    ip += "255";
    udp_server.sin_addr.s_addr = inet_addr(ip.c_str());
    udp_server.sin_port = htons(9);

    // send the packet
    sendto(udp_socket, &to_send, sizeof(unsigned char) * 102, 0, (struct sockaddr*)&udp_server, sizeof(udp_server));

    handleProgressState(100);
}

void Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)
{    
    Logger::getInstance()->debug("Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)");
    TreeWidgetItem *item = (TreeWidgetItem*) m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->getId();
    Logger::getInstance()->info("Connecting to " + ip);

    switch (getSilentConnection(ip, false)) {
    case 0:
        emit doneConnecting(m_connections[ip], state);
        break;
    case 1:
        emit authenticate(state);
        break;
    case -1:
        // error already displayed
        handleProgressState(1);
        break;
    }
}

void Engine::Kernel::loadPlugin()
{
    Logger::getInstance()->debug("Engine::Kernel::loadPlugin()");
    QDir plugins_dir(qApp->applicationDirPath());
    plugins_dir.cd(STR(PLUGIN_PATH));

    foreach (QString file_name, plugins_dir.entryList(QDir::Files)) {
        QPluginLoader *plugin_loader = new QPluginLoader(plugins_dir.absoluteFilePath(file_name));
        QObject *plugin = plugin_loader->instance();
        IPlugin *loaded_plugin = NULL;
        m_loaders.push_back(plugin_loader);
        if (plugin && (loaded_plugin = qobject_cast<IPlugin*>(plugin))) {
            if (m_loaded_plugins.find(file_name.toStdString()) == m_loaded_plugins.end()) {
                Logger::getInstance()->debug("Loaded: " + loaded_plugin->getLabel(), true);
                m_loaded_plugins[file_name.toStdString()] = loaded_plugin;

                loaded_plugin->connectButtons(m_main_window.getToolbar());
                connect(loaded_plugin, SIGNAL(unsavedChanges(IPlugin*)), this, SLOT(setPluginUnsavedChanges(IPlugin*)));
                connect(loaded_plugin, SIGNAL(noChanges(IPlugin*)), this, SLOT(setPluginNoChanges(IPlugin*)));
                connect(loaded_plugin, SIGNAL(refreshProgress(int)), this, SLOT(handleProgressState(int)));
                connect(loaded_plugin, SIGNAL(newInstructionText(std::string)), this, SLOT(handleInstructionText(std::string)));

                m_main_window.getProviderWidget()->getTabWidget()->addTab(loaded_plugin, loaded_plugin->getLabel().c_str());
            }
        } else {
            Logger::getInstance()->error(plugin_loader->errorString().toStdString(), false);
        }
    }
}

void Engine::Kernel::showMainWindow()
{
    Logger::getInstance()->debug("Engine::Kernel::showMainWindow()");
    loadPlugin();
    setActivePlugin(0);
    m_main_window.show();
}
