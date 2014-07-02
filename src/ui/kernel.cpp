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
#include "logger.h"

#include <boost/thread.hpp>
#include <gnome-keyring-1/gnome-keyring.h>
#include <Pegasus/Common/Array.h>

#include <QToolBar>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;

Engine::Kernel::Kernel() :
    m_refreshEnabled(true),
    m_mutex(new QMutex()),
    m_bar(new QProgressBar(m_main_window.getProviderWidget()))
{
    struct passwd *pw = getpwuid(getuid());
    std::string path = pw->pw_dir;
    path += "/.openlmi";
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST) {
        Logger::getInstance()->error("Cannot create ~/.openlmi dir.\nError: " + std::string(strerror(errno)));
    }
    QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(refresh())
        );
    button = m_main_window.getToolbar()->findChild<QPushButton*>("shutdown_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(shutdownPc()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("reboot_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(rebootPc()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("edit_button");
    connect(
        button,
        SIGNAL(toggled(bool)),
        this,
        SLOT(setEditState(bool)));
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
        SLOT(enableSpecialButtons()));
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
    m_bar->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_code_dialog.setTitle("LMIShell Code");
    createKeyring();        

    m_event_log.setConnectionStorage(&m_connections);
    m_event_log.setPCTree(m_main_window.getPcTreeWidget()->getTree());
}

Engine::Kernel::~Kernel()
{
    Logger::removeInstance();
    delete m_mutex;
    delete m_bar;
}

int Engine::Kernel::getIndexOfTab(std::string name)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    for (int i = 0; i < tab->count(); i++) {
       if (tab->tabText(i).contains(name.c_str()))
           return i;
    }

    return -1;
}

void Engine::Kernel::createKeyring()
{
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

void Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    bool refreshed = plugin->isRefreshed();

    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("apply_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("cancel_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_as_button"))->setEnabled(state & refreshed);
    if (refresh_button) {
        enableSpecialButtons(state);
    }    
}

void Engine::Kernel::setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state)
{
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

void Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)
{    
    Logger::getInstance()->info("Connecting...");
    QTreeWidgetItem* item = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->text(0).toStdString();

    switch (getSilentConnection(ip)) {
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
    QDir plugins_dir(qApp->applicationDirPath());
    plugins_dir.cd(PLUGIN_PATH);    

    foreach (QString file_name, plugins_dir.entryList(QDir::Files)) {
        QPluginLoader plugin_loader(plugins_dir.absoluteFilePath(file_name));
        QObject *plugin = plugin_loader.instance();
        IPlugin *loaded_plugin = NULL;
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
            Logger::getInstance()->error(plugin_loader.errorString().toStdString());
        }
    }
}

void Engine::Kernel::showMainWindow()
{
    loadPlugin();
    setActivePlugin(0);
    m_main_window.show();
}
