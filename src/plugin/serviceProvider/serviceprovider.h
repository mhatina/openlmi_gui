/* ***** BEGIN LICENSE BLOCK *****
 *
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 2.1 of the
 *   License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301 USA
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef SERVICEPROVIDER_H
#define SERVICEPROVIDER_H

#include "plugin.h"

#include <QTableWidget>
#include <QtPlugin>
#include <QWidget>

const Engine::property serviceProperties[] = {
    {"Name", "Name", false},
    {"Caption", "Caption", false},
//    {"Enabled default", "EnabledDefault", false},
//    {"Enabled state", "EnabledState", false},
    {"Operational status", "OperationalStatus", false},
//    {"Requested state", "RequestedState", false},
    {"Status", "Status", false}
};

const char* const action_list[] = {
    "-",
    "Reload",
    "Restart",
    "Start",
    "Stop",
    "Enable",
    "Disable"
};

typedef enum {
    NOTHING,
    RELOAD,
    RESTART,
    START,
    STOP,
    ENABLE,
    DISABLE
} e_action;

const char* const enabled_default_values[] = {
    "Enabled",
    "Disabled",
    "Not Applicable",
    "Enabled but Offline",
    "No Default",
    "Quiesce"
};

const char* const enabled_state_values[] = {
    "Unknown",
    "Other",
    "Enabled",
    "Disabled",
    "Shutting Down",
    "Not Applicable",
    "Enabled but Offline",
    "In Test",
    "Deferred",
    "Quiesce",
    "Starting"
};

const char* const operational_status_values[] =  {
    "Unknown",
    "Other",
    "OK",
    "Degraded",
    "Stressed",
    "Predictive Failure",
    "Error",
    "Non-Recoverable Error",
    "Starting",
    "Stopping",
    "Stopped",
    "In Service",
    "No Contact",
    "Lost Communication",
    "Aborted",
    "Dormant",
    "Supporting Entity in Error",
    "Completed",
    "Power Mode",
    "Relocating"
};

const char* const requested_state_values[] = {
    "Unknown",
    "",
    "Enabled",
    "Disabled",
    "Shut Down",
    "No Change",
    "Offline",
    "Test",
    "Deferred",
    "Quiesce",
    "Reboot",
    "Reset",
    "Not Applicable"
};

const Qt::ItemFlags item_flags =
        Qt::ItemIsSelectable |
        Qt::ItemIsEnabled;

namespace Ui {
class ServiceProviderPlugin;
}

/**
 * @brief The ServiceProviderPlugin class
 */
class ServiceProviderPlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)
    
private:
    std::vector<Pegasus::CIMInstance> m_service_instances;
    QTableWidget *m_services_table;
    Ui::ServiceProviderPlugin *m_ui;    

    std::string valueToStr(Pegasus::CIMProperty property);

public:
    /**
     * @brief Constructor
     */
    explicit ServiceProviderPlugin();
    /**
     * @brief Destructor
     */
    ~ServiceProviderPlugin();
    virtual std::string getInstructionText();
    virtual std::string getLabel();    
    virtual void fillTab(std::vector<void *> *data);
    virtual void generateCode();
    virtual void getData(std::vector<void *> *data);

private slots:
    void actionHandle(std::string name, e_action action);
    void showDetails();
};

#endif // SERVICEPROVIDER_H
