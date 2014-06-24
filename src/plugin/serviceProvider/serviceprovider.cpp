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
    return "&Services";
}

void ServiceProviderPlugin::generateCode()
{
    if (!m_active)
        return;
    if (m_instructions.empty())
        return;

    if (m_save_dir_path.empty())
        m_save_dir_path = getPath();

    std::ofstream out_file;
    std::string filename = m_save_dir_path + "/" + m_client->hostname() + "_services";
    out_file.open(filename.c_str());

    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        out_file << m_instructions[i]->toString();
    }

    out_file.close();
}

void ServiceProviderPlugin::getData(std::vector<void *> *data)
{
    Pegasus::Array<Pegasus::CIMInstance> services;
    Pegasus::Array<Pegasus::CIMObject> services_obj;
    std::string filter = m_ui->filter_line->text().toStdString();

    try
    {

        m_client->setTimeout(Pegasus::Uint32(600000));

        if (!filter.empty()) {            
            services_obj = m_client->execQuery(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::String("WQL"),
                        Pegasus::String(std::string("SELECT * FROM LMI_Service WHERE " + filter).c_str())
                        );
        } else {
         services = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_Service"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                    );
        }
    } catch (Pegasus::Exception &ex)
    {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }

    unsigned int cnt = !filter.empty() ? services_obj.size() : services.size();
    for (unsigned int i = 0; i < cnt; i++)
        data->push_back(new Pegasus::CIMInstance(!filter.empty() ? services_obj[i] : services[i]));

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
                Pegasus::CIMValue value = property.getValue();
                std::string str_value;

                if (property.getName().equal(Pegasus::CIMName("EnabledDefault")))
                    str_value = enabled_default_values[atoi(value.toString().getCString())];
                else if (property.getName().equal(Pegasus::CIMName("EnabledState")))
                    str_value = enabled_state_values[atoi(value.toString().getCString())];
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
                    str_value = ss.str();
                } else if (property.getName().equal(Pegasus::CIMName("RequestedState")))
                    str_value = requested_state_values[atoi(value.toString().getCString())];
                else
                    str_value = CIMValue::to_std_string(value);

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

Q_EXPORT_PLUGIN2(serviceProvider, ServiceProviderPlugin)
