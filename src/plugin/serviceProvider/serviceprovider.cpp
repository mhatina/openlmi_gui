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

#include "actionbox.h"
#include "detailsdialog.h"
#include "instructions/disableserviceinstruction.h"
#include "instructions/enableserviceinstruction.h"
#include "instructions/getinstruction.h"
#include "instructions/reloadserviceinstruction.h"
#include "instructions/restartserviceinstruction.h"
#include "instructions/startserviceinstruction.h"
#include "instructions/stopserviceinstruction.h"
#include "lmiwbem_value.h"
#include "serviceprovider.h"
#include "ui_serviceprovider.h"

#include <sstream>
#include <vector>

std::string ServiceProviderPlugin::valueToStr(Pegasus::CIMProperty property)
{
    Pegasus::CIMValue value = property.getValue();
    if (property.getName().equal(Pegasus::CIMName("EnabledDefault")))
        return enabled_default_values[atoi(value.toString().getCString())];
    else if (property.getName().equal(Pegasus::CIMName("EnabledState")))
        return enabled_state_values[atoi(value.toString().getCString())];
    else if (property.getName().equal(Pegasus::CIMName("OperationalStatus"))) {
        Pegasus::Array<Pegasus::Uint16> raw_array;
        std::stringstream ss;
        value.get(raw_array);
        const Pegasus::Uint32 cnt = value.getArraySize();
        for (Pegasus::Uint32 i = 0; i < cnt; ++i) {
            const Pegasus::Uint16 &raw_value = raw_array[i];
            ss << operational_status_values[atoi(CIMValue::to_std_string(raw_value).c_str())];
            if (i < cnt - 1)
                ss << ", ";
        }
        return ss.str();
    } else if (property.getName().equal(Pegasus::CIMName("RequestedState"))
               || property.getName().equal(Pegasus::CIMName("TransitioningToState")))
        return requested_state_values[atoi(value.toString().getCString())];
    else
        return CIMValue::to_std_string(value);
}

ServiceProviderPlugin::ServiceProviderPlugin() :
    IPlugin(),
    m_ui(new Ui::ServiceProviderPlugin)
{
    m_ui->setupUi(this);
    m_services_table = m_ui->services_table;
    int prop_cnt = sizeof(serviceProperties) / sizeof(serviceProperties[0]);
    m_services_table->setColumnCount(prop_cnt + 1);
    for (int i = 0; i < prop_cnt; i++)
        m_services_table->setHorizontalHeaderItem(
                    i,
                    new QTableWidgetItem(serviceProperties[i].display_name)
                    );
    m_services_table->setHorizontalHeaderItem(
                prop_cnt,
                new QTableWidgetItem("Action")
                );
    m_services_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    connect(
        m_ui->services_table,
        SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
        this,
        SLOT(showDetails()));
    showFilter(false);
    setPluginEnabled(false);
}

ServiceProviderPlugin::~ServiceProviderPlugin()
{
    delete m_ui;
}

std::string ServiceProviderPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string ServiceProviderPlugin::getLabel() 
{
    return "Services";
}

std::string ServiceProviderPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << m_service_instances.size() << " service(s) shown";
    return ss.str();
}

void ServiceProviderPlugin::getData(std::vector<void *> *data)
{
    Pegasus::Array<Pegasus::CIMInstance> services;
    std::string filter = m_ui->filter_line->text().toStdString();

    try
    {
        m_client->setTimeout(Pegasus::Uint32(600000));

        services = m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_Service"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
                );
    } catch (Pegasus::Exception &ex)
    {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }

    unsigned int cnt = services.size();
    for (unsigned int i = 0; i < cnt; i++) {
        Pegasus::CIMInstance instance;
        if (!filter.empty()) {
            instance = services[i];
            if (getPropertyOfInstance(instance, "Name").find(filter) == std::string::npos)
                continue;
        } else
            instance = services[i];

        data->push_back(new Pegasus::CIMInstance(instance));
        m_service_instances.push_back(instance);
    }

    emit doneFetchingData(data);
}

void ServiceProviderPlugin::fillTab(std::vector<void *> *data)
{
    m_services_table->setRowCount(0);

    try
    {
        Pegasus::Uint32 services_cnt = data->size();
        for (unsigned int i = 0; i < services_cnt; i++) {
            int row_count = m_services_table->rowCount();
            m_services_table->insertRow(row_count);

            int prop_cnt = sizeof(serviceProperties) / sizeof(serviceProperties[0]);
            std::string serv_name;
            for (int j = 0; j < prop_cnt; j++) {
                Pegasus::Uint32 propIndex = ((Pegasus::CIMInstance *) (*data)[i])->findProperty(Pegasus::CIMName(serviceProperties[j].property));
                if (propIndex == Pegasus::PEG_NOT_FOUND) {
                    Logger::getInstance()->error("property " + std::string(serviceProperties[j].property) + " not found");
                    continue;
                }

                Pegasus::CIMProperty property = ((Pegasus::CIMInstance *) (*data)[i])->getProperty(propIndex);                
                std::string str_value;

                str_value = valueToStr(property);

                if (property.getName().equal(Pegasus::CIMName("Name")))
                    serv_name = str_value;

                QTableWidgetItem *item =
                        new QTableWidgetItem(str_value.c_str());

                item->setToolTip(str_value.c_str());
                m_services_table->setItem(
                            row_count,
                            j,
                            item
                            );

                if (!serviceProperties[j].enabled)
                    m_services_table->item(row_count, j)->setFlags(item_flags);
            }

            ActionBox *box = new ActionBox(serv_name);
            connect(
                box,
                SIGNAL(performAction(std::string,e_action)),
                this,
                SLOT(actionHandle(std::string,e_action)));
            m_services_table->setCellWidget(
                        row_count,
                        prop_cnt,
                        box);
        }

        m_services_table->sortByColumn(0, Qt::AscendingOrder);
    } catch (Pegasus::Exception &ex)
    {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }

    for (unsigned int i = 0; i < data->size(); i++)
        delete ((Pegasus::CIMInstance *)(*data)[i]);
}

void ServiceProviderPlugin::actionHandle(std::string name, e_action action)
{
    addInstruction(new GetInstruction(name));

    switch (action) {
    case RELOAD:
        addInstruction(new ReloadServiceInstruction(m_client, name));
        break;
    case RESTART:
        addInstruction(new RestartServiceInstruction(m_client, name));
        break;
    case START:
        addInstruction(new StartServiceInstruction(m_client, name));
        break;
    case STOP:
        addInstruction(new StopServiceInstruction(m_client, name));
        break;
    case ENABLE:
        addInstruction(new EnableServiceInstruction(m_client, name));
        break;
    case DISABLE:
        addInstruction(new DisableServiceInstruction(m_client, name));
        break;
    default:
        return;
    }
}

void ServiceProviderPlugin::showDetails()
{
    Pegasus::CIMInstance service;
    std::string name_expected = m_ui->services_table->selectedItems()[0]->text().toStdString();
    int cnt = m_service_instances.size();
    for (int i = 0; i < cnt; i++) {
        if (name_expected == getPropertyOfInstance(m_service_instances[i], "Name"))
            service = m_service_instances[i];
    }

    std::map<std::string, std::string> values;
    cnt = service.getPropertyCount();
    for (int i = 0; i < cnt; i++) {
        std::string object_name = std::string(service.getProperty(i).getName().getString().getCString());
        std::string str_value = valueToStr(service.getProperty(i));
        values[object_name] = str_value;
    }

    DetailsDialog dialog("Service details", this);
    dialog.setValues(values, true);
    dialog.hideCancelButton();
    dialog.exec();
}

Q_EXPORT_PLUGIN2(serviceProvider, ServiceProviderPlugin)
