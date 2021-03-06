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

#include "overview.h"
#include "instructions/createloginstruction.h"
#include "cimvalue.h"
#include "logseveritydialog.h"
#include "ui_overview.h"
#include "widgets/labeledlabel.h"

#include <Pegasus/Common/Array.h>
#include <sstream>

String OverviewPlugin::decode(Pegasus::CIMProperty property)
{
    Pegasus::CIMValue value = property.getValue();
    if (property.getName().equal(Pegasus::CIMName("PowerState"))) {
        return power_state_values[atoi(CIMValue::to_string(value))];
    } else if (property.getName().equal(Pegasus::CIMName("SyslogSeverity"))) {
        return syslog_severity_values[atoi(CIMValue::to_string(value))];
    } else if (property.getName().equal(Pegasus::CIMName("PerceivedSeverity"))) {
        return perceived_severity_values[atoi(CIMValue::to_string(value))];
    }

    return "";
}

String OverviewPlugin::getTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%G%m%d000000", timeinfo);

    return buffer;
}

void OverviewPlugin::fillLogBox(String filter)
{
    m_log_mutex->lock();
    unsigned int cnt = m_logs.size();
    m_log_mutex->unlock();
    for (unsigned int i = 0; i < cnt; i++) {
        boost::this_thread::interruption_point();
        if (i % 50 == 0) {
            usleep(500000);    // half second
        }
        boost::this_thread::interruption_point();

        String title;
        for (int j = 0; ; j++) {
            m_log_mutex->lock();
            if (m_logs.empty()) {
                m_log_mutex->unlock();
                return;
            }
            Pegasus::Uint32 ind;
            switch (j) {
            case 0:
                ind = m_logs[i].findProperty("SyslogSeverity");
                title = decode(m_logs[i].getProperty(ind));
                break;
            case 1:
                ind = m_logs[i].findProperty("PerceivedSeverity");
                title = decode(m_logs[i].getProperty(ind));
                break;
            default:
                title = "Unknown";
                break;
            }
            m_log_mutex->unlock();

            if (!title.empty()) {
                break;
            }
        }

        boost::this_thread::interruption_point();
        m_log_mutex->lock();
        String log = CIMValue::get_property_value(m_logs[i], "DataFormat");
        if (filter.empty() || log.find(filter) != String::npos ||
            title.find(filter) != String::npos) {
            emit addLog(title, log);
        }
        m_log_mutex->unlock();
    }
}

OverviewPlugin::OverviewPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_journald_available(false),
    m_battery_available(true),
    m_log_thread(NULL),
    m_log_mutex(new QMutex()),
    m_ui(new Ui::OverviewPlugin)
{
    m_ui->setupUi(this);
    m_ui->filter_box->hide();
    setPluginEnabled(false);
    m_ui->new_log_entry->setFocus(Qt::ActiveWindowFocusReason);

    connect(
        m_ui->filter,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(filterChanged(QString)));
    connect(
        this,
        SIGNAL(addLog(String, String)),
        this,
        SLOT(addLogEntry(String, String)));
    connect(
        m_ui->add_new_log_button,
        SIGNAL(clicked()),
        this,
        SLOT(showLogSeverity()));
}

OverviewPlugin::~OverviewPlugin()
{
    delete m_ui;
}

void OverviewPlugin::addLogEntry(String label, String text)
{
    if (!m_ui->content->layout()) {
        m_ui->content->setLayout(new QGridLayout());
    }

    QLayout *l = m_ui->content->layout();
    l->addWidget(new LabeledLabel(label, text));
}

void OverviewPlugin::filterChanged(QString text)
{
    if (m_log_thread != NULL) {
        m_log_thread->interrupt();
        delete m_log_thread;
    }

    QObjectList list = m_ui->content->children();
    for (int i = list.size() - 1; i >= 0; i--) {
        QLayout *l = m_ui->content->layout();
        l->removeWidget(qobject_cast<QWidget *>(list[i]));
        delete list[i];
    }

    m_log_thread = new boost::thread(boost::bind(&OverviewPlugin::fillLogBox, this,
                                     text.toStdString()));
}

void OverviewPlugin::showLogSeverity()
{
    LogSeverityDialog dialog(this);
    dialog.setMessage(m_ui->new_log_entry->text().toStdString());
    if (dialog.exec()) {
        addInstruction(
            new CreateLogInstruction(
                m_client,
                dialog.getMessage(),
                dialog.getSeverity()));
    }
}

String OverviewPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

String OverviewPlugin::getLabel()
{
    return "Overview";
}

String OverviewPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": ";
    if (m_journald_available) {
        ss << m_logs.size() << " log records from today shown";
    } else {
        ss << "Journald provider not available";
    }

    return ss.str();
}

void OverviewPlugin::clear()
{
    m_changes_enabled = false;

    m_ui->name->setText("N/A");
    m_ui->description->setText("N/A");
    m_ui->power_state->setText("N/A");

    QObjectList list = m_ui->power_box->children();
    for (int i = list.size() - 1; i >= 0; i--) {
        m_ui->power_box->layout()->removeWidget(qobject_cast<QWidget *>(list[i]));
        delete list[i];
    }
    list = m_ui->network_box->children();
    for (int i = list.size() - 1; i >= 0; i--) {
        m_ui->network_box->layout()->removeWidget(qobject_cast<QWidget *>(list[i]));
        delete list[i];
    }
    list = m_ui->content->children();
    for (int i = list.size() - 1; i >= 0; i--) {
        QLayout *l = m_ui->content->layout();
        l->removeWidget(qobject_cast<QWidget *>(list[i]));
        delete list[i];
    }
    m_logs.clear();
    m_ui->log_box->setEnabled(false);

    m_changes_enabled = true;
}

void OverviewPlugin::getData(std::vector<void *> *data)
{
    try {
        Pegasus::Array<Pegasus::CIMInstance> provider =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/interop"),
                Pegasus::CIMName("PG_RegisteredProfile"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );

        for (unsigned int i = 0; i < provider.size(); i++) {
            m_journald_available = CIMValue::get_property_value(provider[i],
                                   "RegisteredName").find("Journald") != String::npos;
            if (m_journald_available) {
                break;
            }
        }

        std::vector<void *> *tmp = new std::vector<void *>();
        Pegasus::Array<Pegasus::CIMInstance> system =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("PG_ComputerSystem"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );

        if (system.size() != 1) {
            emit doneFetchingData(NULL, false);
            return;
        }

        tmp->push_back(new Pegasus::CIMInstance(system[0]));
        emit doneFetchingData(tmp , true);

        Pegasus::Array<Pegasus::CIMInstance> battery =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_Battery"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );

        if ((m_battery_available = battery.size()) != 0) {
            tmp = new std::vector<void *>();
            for (unsigned int i = 0; i < battery.size(); i++) {
                tmp->push_back(new Pegasus::CIMInstance(battery[i]));
            }
            emit doneFetchingData(tmp, true);
        }

        Pegasus::Array<Pegasus::CIMInstance> network =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_IPNetworkConnection"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );
        for (unsigned int i = 0; i < network.size(); i++) {
            if (CIMValue::get_property_value(network[i], "OperatingStatus") != "16") { // In service
                continue;
            }

            Pegasus::Array<Pegasus::CIMObject> ip =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    network[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_IPProtocolEndpoint")
                );

            tmp = new std::vector<void *>();
            for (unsigned int j = 0; j < ip.size(); j++) {
                tmp->push_back(new Pegasus::CIMInstance(ip[j]));
            }
            emit doneFetchingData(tmp, true);
        }

        emit refreshProgress(Engine::ALMOST_REFRESHED, this);

        if (!m_journald_available) {
            emit doneFetchingData(data);
            return;
        }

        String time = getTime();
        String query = "Select * FROM LMI_JournalLogRecord WHERE MessageTimestamp > \"";
        query += time + "\"";
        Pegasus::Array<Pegasus::CIMObject> log =
                execQuery(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::String("WQL"),
                    query
                    );

        for (unsigned int i = 0; i < log.size(); i++) {
            data->push_back(new Pegasus::CIMInstance(log[i]));
            m_log_mutex->lock();
            m_logs.push_back(Pegasus::CIMInstance(log[i]));
            m_log_mutex->unlock();
        }

    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, CIMValue::to_string(ex.getMessage()));
        return;
    }

    emit doneFetchingData(data);
}

void OverviewPlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    try {
        int battery_cnt = 1;
        for (unsigned int i = 0; i < data->size(); i++) {
            Pegasus::CIMInstance instance = *((Pegasus::CIMInstance *) (*data)[i]);

            if (CIMValue::get_property_value(instance, "CreationClassName") == "PG_ComputerSystem") {
                m_ui->name->setText(CIMValue::get_property_value(instance, "Name"));
                m_ui->description->setText(CIMValue::get_property_value(instance, "Description"));

                Pegasus::Uint32 prop_ind = instance.findProperty("PowerState");
                m_ui->power_state->setText(decode(instance.getProperty(prop_ind)));
            } else if (CIMValue::get_property_value(instance,
                                                    "CreationClassName") == "LMI_Battery") {
                if (!m_ui->power_box->layout()) {
                    m_ui->power_box->setLayout(new QFormLayout());
                    m_ui->power_box->layout()->setContentsMargins(2, 9, 2, 9);
                }
                if (battery_cnt > 1) {
                    QFrame *line = new QFrame();
                    line->setFrameShape(QFrame::HLine);
                    line->setFrameShadow(QFrame::Sunken);
                    m_ui->power_box->layout()->addWidget(line);
                }

                std::stringstream title;
                title << "Battery #" << battery_cnt << ":";
                String text = CIMValue::get_property_value(instance,
                                   "EstimatedChargeRemaining");
                // charge is 100 - EstimatedChargeRemaining

                m_ui->power_box->layout()->addWidget(new LabeledLabel(
                        title.str(),
                        (text.empty() ? "N/A" : text + "%")));

                text = CIMValue::get_property_value(instance, "EstimatedRunTime");
                m_ui->power_box->layout()->addWidget(new LabeledLabel(
                        "Est. time:",
                        (text.empty() ? "N/A" : text + " min")));

                text = CIMValue::get_property_value(instance, "BatteryStatus");
                m_ui->power_box->layout()->addWidget(new LabeledLabel(
                        "Battery status:",
                        text));

                battery_cnt++;
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_IPProtocolEndpoint") {
                if (!m_ui->network_box->layout()) {
                    m_ui->network_box->setLayout(new QFormLayout());
                    m_ui->network_box->layout()->setContentsMargins(2, 9, 2, 9);
                }

                String title = "IPv4";
                String ip = CIMValue::get_property_value(instance, "IPv4Address");
                if (ip.empty()) {
                    ip = CIMValue::get_property_value(instance, "IPv6Address");
                    title = "IPv6";
                }
                m_ui->network_box->layout()->addWidget(new LabeledLabel(
                        title,
                        ip));
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_JournalLogRecord") {
                filterChanged("");
                break;
            }
        }

        if (!m_battery_available) {
            String status = "A/C";
            if (!m_ui->power_box->layout()) {
                m_ui->power_box->setLayout(new QFormLayout());
                m_ui->power_box->layout()->setContentsMargins(2, 9, 2, 9);
            }
            m_ui->power_box->layout()->addWidget(new LabeledLabel(
                    "Status:",
                    status));
            m_battery_available = true;
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_string(ex.getMessage()));
        return;
    }

    for (unsigned int i = 0; i < data->size(); i++) {
        delete (Pegasus::CIMInstance *) (*data)[i];
    }

    if (!m_journald_available) {
        m_ui->log_box->setEnabled(false);
    }

    m_changes_enabled = true;
}

Q_EXPORT_PLUGIN2(overview, OverviewPlugin)
