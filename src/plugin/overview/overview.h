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

#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "widgets/labeledlabel.h"
#include "plugin.h"

#include <QtPlugin>

const char *const power_state_values[] = {
    "Unknown",
    "Full Power",
    "Power Save - Low Power Mode",
    "Power Save - Standby",
    "Power Save - Unknown",
    "Power Cycle", "Power Off",
    "Power Save - Warning",
    "Power Save - Hibernate",
    "Power Save - Soft Off"
};

const char *const syslog_severity_values[] = {
    "Emergency",
    "Alert",
    "Critical",
    "Error",
    "Warning",
    "Notice",
    "Informational",
    "Debug"
};

const char *const perceived_severity_values[] = {
    "Unknown",
    "Other",
    "Information",
    "Degraded/Warning",
    "Minor",
    "Major",
    "Critical",
    "Fatal/NonRecoverable"
};

namespace Ui
{
class OverviewPlugin;
}

class OverviewPlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    bool m_changes_enabled;
    bool m_journald_available;
    bool m_battery_available;
    boost::thread *m_log_thread;
    std::vector<Pegasus::CIMInstance> m_logs;
    QMutex *m_log_mutex;
    Ui::OverviewPlugin *m_ui;

    std::string decode(Pegasus::CIMProperty property);
    std::string getTime();
    void fillLogBox(std::string filter);

private slots:
    void addLogEntry(std::string label, std::string text);
    void filterChanged(QString text);
    void showLogSeverity();

public:
    explicit OverviewPlugin();
    ~OverviewPlugin();
    virtual std::string getInstructionText();
    virtual std::string getLabel();
    virtual std::string getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

signals:
    void addLog(std::string label, std::string text);
};

#endif // OVERVIEW_H
