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

#include "hardwareprovider.h"
#include "ui_hardwareprovider.h"

#include <sstream>
#include <vector>

HardwareProviderPlugin::HardwareProviderPlugin() :
    IPlugin(),
    m_ui(new Ui::HardwareProviderPlugin)
{
    m_ui->setupUi(this);
    showFilter(false);
    setPluginEnabled(false);
}

HardwareProviderPlugin::~HardwareProviderPlugin()
{
    delete m_ui;
}

std::string HardwareProviderPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string HardwareProviderPlugin::getLabel()
{
    return "&Hardware";
}

void HardwareProviderPlugin::getData(std::vector<void *> *data)
{
//    Pegasus::Array<Pegasus::CIMInstance> services;
//    Pegasus::Array<Pegasus::CIMObject> services_obj;
//    std::string filter = m_ui->filter_line->text().toStdString();

//    try
//    {
//        if (!filter.empty()) {
//            services_obj = m_client->execQuery(
//                        Pegasus::CIMNamespaceName("root/cimv2"),
//                        Pegasus::String("WQL"),
//                        Pegasus::String(std::string("SELECT * FROM LMI_Service WHERE " + filter).c_str())
//                        );
//        } else {
//         services = m_client->enumerateInstances(
//                    Pegasus::CIMNamespaceName("root/cimv2"),
//                    Pegasus::CIMName("LMI_Service"),
//                    true,       // deep inheritance
//                    false,      // local only
//                    false,      // include qualifiers
//                    false       // include class origin
//                    );
//        }
//    } catch (Pegasus::Exception &ex)
//    {
//        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
//        return;
//    }

//    unsigned int cnt = !filter.empty() ? services_obj.size() : services.size();
//    for (unsigned int i = 0; i < cnt; i++) {
//        Pegasus::CIMInstance instance = !filter.empty() ? Pegasus::CIMInstance(services_obj[i]) : services[i];
//        data->push_back(new Pegasus::CIMInstance(instance));
//        m_service_instances.push_back(instance);
//    }

//    emit doneFetchingData(data);
}

void HardwareProviderPlugin::fillTab(std::vector<void *> *data)
{

}

Q_EXPORT_PLUGIN2(hardwareProvider, HardwareProviderPlugin)
