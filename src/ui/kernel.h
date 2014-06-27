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
#include "config.h"
#include "eventlog.h"
#include "lmiwbem_client.h"
#include "mainwindow.h"
#include "plugin.h"

#include <map>
#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <string>

namespace PowerStateValues {
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
    PowerCycleOffHardGraceful = 16
} POWER_VALUES;
} // end of namespace PowerStateValues

namespace Engine {
/**
* @brief The Kernel class.
*
* Basic class of OpenLMI gui.
*/
class Kernel : public QObject {
    Q_OBJECT

    typedef std::map<std::string, CIMClient*> connection_map;
    typedef std::map<std::string, IPlugin*> plugin_map;

private:    
    bool m_refreshEnabled;
    connection_map m_connections;
    EventLog m_event_log;
    MainWindow m_main_window;
    plugin_map m_loaded_plugins;
    QMutex *m_mutex;
    QProgressBar *m_bar;
    ShowTextDialog m_code_dialog;

    int getIndexOfTab(std::string name);
    void createKeyring();
    /**
     * @brief setButtonsEnabled -- enable/disable buttons
     * @param state
     * @param refresh_button -- affect refresh_button?
     */
    void setButtonsEnabled(bool state, bool refresh_button = true);
    void setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state);

public:
    /**
     * @brief Constructor
     */
    Kernel();
    /**
     * @brief Destructor
     */
    ~Kernel();
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

private slots:    
    int getSilentConnection(std::string ip);
    void deletePasswd();
    void deletePasswd(std::string id);
    void enableSpecialButtons();
    void enableSpecialButtons(bool state);
    void handleAuthentication(PowerStateValues::POWER_VALUES state);
    void handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state);
    void handleError(std::string message);
    void handleInstructionText(std::string text);
    void handleProgressState(int state);    
    void rebootPc();
    void refresh();
    void resetKeyring();
    void setActivePlugin(int index);
    void setEditState(bool state);
    void setPluginNoChanges(IPlugin *plugin);
    void setPluginUnsavedChanges(IPlugin *plugin);
    void showCodeDialog();
    void showFilter();
    void shutdownPc();    

signals:
    /**
     * @brief Is emitted when prompt for authentication is necessary
     * @param state -- power state
     */
    void authenticate(PowerStateValues::POWER_VALUES state);
    /**
     * @brief Error log
     * @param message -- text that is logged
     */
    void error(std::string message);
    /**
     * @brief Is emitted when client is connected to host
     * @param client -- representation of connection
     * @param state -- power state
     */
    void doneConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state);
};
} // namespace Engine

#endif // KERNEL_H
