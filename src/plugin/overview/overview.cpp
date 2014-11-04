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

std::string OverviewPlugin::decode(Pegasus::CIMProperty property)
{
    Pegasus::CIMValue value = property.getValue();
    if (property.getName().equal(Pegasus::CIMName("PowerState"))) {
        return power_state_values[atoi(CIMValue::to_std_string(value).c_str())];
    }

    return "";
}

void OverviewPlugin::fillLogBox(std::string filter)
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

        std::string title;
        for (int j = 0; ; j++) {
            m_log_mutex->lock();
            if (m_logs.empty()) {
                m_log_mutex->unlock();
                return;
            }
            switch (j) {
            case 0:
                title = CIMValue::get_property_value(m_logs[i], "SyslogSeverity");
                break;
            case 1:
                title = CIMValue::get_property_value(m_logs[i], "PerceivedSeverity");
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
        std::string log = CIMValue::get_property_value(m_logs[i], "DataFormat");
        if (filter.empty() || log.find(filter) != std::string::npos ||
            title.find(filter) != std::string::npos) {
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

    connect(
        m_ui->filter,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(filterChanged(QString)));
    connect(
        this,
        SIGNAL(addLog(std::string, std::string)),
        this,
        SLOT(addLogEntry(std::string, std::string)));
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

void OverviewPlugin::addLogEntry(std::string label, std::string text)
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

std::string OverviewPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string OverviewPlugin::getLabel()
{
    return "Overview";
}

std::string OverviewPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": ";
    if (m_journald_available) {
        ss << m_logs.size() << " log records shown";
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
                                   "RegisteredName").find("Journald") != std::string::npos;
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

        Pegasus::Array<Pegasus::CIMInstance> log =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_JournalLogRecord"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );

        for (unsigned int i = 0; i < log.size(); i++) {
            data->push_back(new Pegasus::CIMInstance(log[i]));
            m_log_mutex->lock();
            m_logs.push_back(log[i]);
            m_log_mutex->unlock();
        }

    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, CIMValue::to_std_string(ex.getMessage()));
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
                m_ui->name->setText(CIMValue::get_property_value(instance, "Name").c_str());
                m_ui->description->setText(CIMValue::get_property_value(instance,
                                           "Description").c_str());

                Pegasus::Uint32 prop_ind = instance.findProperty("PowerState");
                m_ui->power_state->setText(decode(instance.getProperty(prop_ind)).c_str());
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
                std::string text = CIMValue::get_property_value(instance,
                                   "EstimatedChargeRemaining");
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

                std::string title = "IPv4";
                std::string ip = CIMValue::get_property_value(instance, "IPv4Address");
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
            std::string status = "A/C";
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
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    for (unsigned int i = 0; i < data->size(); i++) {
        delete (Pegasus::CIMInstance *) (*data)[i];
    }

    m_changes_enabled = true;
}

Q_EXPORT_PLUGIN2(overview, OverviewPlugin)
