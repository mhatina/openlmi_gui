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

#ifndef SERVICE_H
#define SERVICE_H

#include "plugin.h"

#include <QTableWidget>
#include <QtPlugin>
#include <QWidget>

const Engine::property serviceProperties[] = {
    {"Name", false},
    {"Caption", false},
    {"OperationalStatus", false},
    {"Status", false}
};

const Engine::header_item serviceHeaderItem[] = {
    {"Name", "The Name property uniquely identifies the Service and provides an indication of the functionality that is managed."},
    {"Caption", "The Caption property is a short textual description (one- line string) of the object."},
    {"Operational status", "Indicates the current statuses of the element."},
    {"Status", "A string indicating the current status of the object."},
    {"Action", "Possible actions that could be performed on service."}
};

const char *const action_list[] = {
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

const Qt::ItemFlags item_flags =
    Qt::ItemIsSelectable |
    Qt::ItemIsEnabled;

namespace Ui
{
class ServicePlugin;
}

/**
 * @brief The ServicePlugin class
 */
class ServicePlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    std::vector<Pegasus::CIMInstance> m_service_instances;
    QTableWidget *m_services_table;
    Ui::ServicePlugin *m_ui;

public:
    /**
     * @brief Constructor
     */
    explicit ServicePlugin();
    /**
     * @brief Destructor
     */
    ~ServicePlugin();
    virtual std::string getInstructionText();
    virtual std::string getLabel();
    virtual std::string getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

private slots:
    void actionHandle(std::string name, e_action action);
    void showDetails();
};

#endif // SERVICE_H
