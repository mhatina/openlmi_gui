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

#ifndef KERNEL_H
#define KERNEL_H

#include "authenticationdialog.h"
#include "cimclient.h"
#include "mainwindow.h"
#include "plugin.h"
#include "settingsdialog.h"
#include "showtextdialog.h"
#include "widgets/progressbar.h"

#include <map>
#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <string>

#define UNUSED(x) (void)x;
#define STR(X)  __STRING(X)

#define OPENLMI_KEYRING_DEFAULT "openlmi"
#define LMI_VERSION "0.1.1"

namespace PowerStateValues
{
typedef enum {
    NoPowerSetting = -1,
    PowerOn = 2,
    SleepLight = 3,
    SleepDeep = 4,
    PowerCycleOffSoft = 5,
    PowerOffHard = 6,
    PowerOffSoft = 8,
    Hibernate = 7,
    PowerCycleOffHard = 9,
    MasterBusReset = 10,
    DiagnosticInterruptNMI = 11,
    PowerOffSoftGraceful = 12,
    PowerOffHardGraceful = 13,
    MasterBusResetGraceful = 14,
    PowerCycleOffSoftGraceful = 15,
    PowerCycleOffHardGraceful = 16,
    WakeOnLan = 17
} POWER_VALUES;
} // end of namespace PowerStateValues

namespace Engine
{
typedef enum {
    ERROR = -1,
    NOT_REFRESHED = 0,
    ALMOST_REFRESHED = 90,
    REFRESHED = 100
} refreshState;

class IPlugin;

/**
* @brief The Kernel class.
*
* Basic class of OpenLMI gui.
*/
class Kernel : public QObject
{
    Q_OBJECT

    typedef std::map<std::string, CIMClient *> connection_map;
    typedef std::map<std::string, IPlugin *> plugin_map;

private:
    bool m_refreshEnabled;
    connection_map m_connections;    
    MainWindow m_main_window;
    plugin_map m_loaded_plugins;
    QMutex *m_mutex;
    ProgressBar *m_bar;
    QTreeWidgetItem *m_last_system;
    SettingsDialog *m_settings;
    ShowTextDialog m_code_dialog;
    std::string m_save_script_path;
    std::vector<QPluginLoader *> m_loaders;

    int  getIndexOfTab(std::string name);
    std::string getPowerStateMessage(PowerStateValues::POWER_VALUES state);
    void createKeyring();
    void initConnections();
    /**
     * @brief setButtonsEnabled -- enable/disable buttons
     * @param state
     * @param refresh_button -- affect refresh_button?
     */
    void setButtonsEnabled(bool state, bool refresh_button = true);
    void setMac(CIMClient *client);
    void setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state);
    void wakeOnLan();

public:
    /**
     * @brief Constructor
     */
    Kernel();
    /**
     * @brief Destructor
     */
    ~Kernel();
    MainWindow *getMainWindow()
    {
        return &m_main_window;
    }

    void deletePlugins();
    /**
     * @brief Connects to system
     * @param state -- power state
     */
    void getConnection(PowerStateValues::POWER_VALUES state);
    /**
     * @brief Load all plugins
     */
    void loadPlugin();
    /**
     * @brief Display main window
     */
    void showMainWindow();

    template<typename T>
    T widget(std::string object_name)
    {
        return m_main_window.findChild<T>(object_name.c_str());
    }

private slots:
    int  getSilentConnection(std::string ip, bool silent = true);
    void deletePasswd();
    void deletePasswd(std::string id);
    void enableSpecialButtons(bool state);
    void handleAuthentication(PowerStateValues::POWER_VALUES state);
    void handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state);
    void handleInstructionText(std::string text);
    void handleProgressState(int state, IPlugin *plugin = NULL);
    void handleProgressState(int state, std::string process, IPlugin *plugin = NULL);
    void refresh();
    void reloadPlugins();
    void reportBug();
    void resetKeyring();
    void saveAsScripts();
    void saveScripts();
    void selectionChanged();
    void setActivePlugin(int index);
    void setPluginNoChanges(IPlugin *plugin);
    void setPluginUnsavedChanges(IPlugin *plugin);
    void setPowerState(QAction *action);
    void showAboutDialog();
    void showCodeDialog();
    void showFilter();
    void showSettings();
    void startLMIShell();
    void startSsh();
    void stopRefresh();

signals:
    /**
     * @brief Is emitted when prompt for authentication is necessary
     * @param state -- power state
     */
    void authenticate(PowerStateValues::POWER_VALUES state);
    /**
     * @brief Is emitted when client is connected to host
     * @param client -- representation of connection
     * @param state -- power state
     */
    void doneConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state);
};
} // namespace Engine

#endif // KERNEL_H
