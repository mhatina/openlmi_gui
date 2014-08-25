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

#include "networkprovider.h"
#include "lmiwbem_value.h"
#include "ui_networkprovider.h"

#include <sstream>

NetworkProviderPlugin::NetworkProviderPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::NetworkProviderPlugin)
{
    m_ui->setupUi(this);
    showFilter(false);
    setPluginEnabled(false);

    m_ui->networks_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_ui->interfaces_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

NetworkProviderPlugin::~NetworkProviderPlugin()
{
    delete m_ui;
}

std::string NetworkProviderPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string NetworkProviderPlugin::getLabel()
{
    return "Network";
}

std::string NetworkProviderPlugin::getRefreshInfo()
{
    std::stringstream ss;
    return ss.str();
}

void NetworkProviderPlugin::getData(std::vector<void *> *data)
{
    try {
    // LMI_IPNetworkConnection LMI_LANEndpoint LMI_NetworkRemoteServiceAccessPoint LMI_IPProtocolEndpoint
        // LMI_EthernetPort LMI_EthernetPortStatistics

    // LMI_IPNetworkConnectionCapabilities
    // LMI_IPNetworkConnectionElementCapabilities
    // LMI_IPVersionSettingData
    // LMI_NetworkHostedAccessPoint
    // LMI_NetworkSystemDevice
    // LMI_NetworkDeviceSAPImplementation
    // LMI_BindsToLANEndpoint
    // LMI_EndpointForIPNetworkConnection
    // LMI_IPElementSettingData
    // LMI_IPVersionElementSettingData
    // LMI_IPAssignmentSettingData
    // LMI_DHCPSettingData
    // LMI_OrderedIPAssignmentComponent
    // LMI_ExtendedStaticIPAssignmentSettingData
    // LMI_IPConfigurationService
    // LMI_HostedIPConfigurationService
    // LMI_IPConfigurationServiceAffectsElement
    // LMI_NetworkRemoteAccessAvailableToElement
    // LMI_DNSProtocolEndpoint
    // LMI_NetworkSAPSAPDependency
    // LMI_DNSSettingData
    // LMI_NetworkEnabledLogicalElementCapabilities
    // LMI_NetworkElementCapabilities
    // LMI_LAGPort8023ad
    // LMI_LinkAggregator8023ad
    // LMI_LinkAggregationBindsTo
    // LMI_LinkAggregationConcreteIdentity
    // LMI_NextHopIPRoute
    // LMI_RouteUsesEndpoint
    // LMI_IPRouteSettingData
    // LMI_SwitchPort
    // LMI_EndpointIdentity
    // LMI_SwitchService
    // LMI_SwitchesAmong
    // LMI_BridgingMasterSettingData
    // LMI_BridgingSlaveSettingData
    // LMI_BondingMasterSettingData
    // LMI_BondingSlaveSettingData
    // LMI_NetworkInstCreation
    // LMI_NetworkInstModification
    // LMI_NetworkInstDeletion
    // LMI_NetworkJob
    // LMI_OwningNetworkJobElement
    // LMI_AffectedNetworkJobElement        
        Pegasus::Array<Pegasus::CIMInstance> network =
                m_client->enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("LMI_IPNetworkConnection"),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                    );

        int cnt = network.size();
        for (int i = 0; i < cnt; i++) {
            std::vector<Pegasus::CIMInstance> *vector = new std::vector<Pegasus::CIMInstance>();
            data->push_back(vector);

            Pegasus::Array<Pegasus::CIMObject> net_assoc =
                    m_client->associators(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        network[i].getPath()
                        );

            vector->push_back(network[i]);
            int cnt_assoc = net_assoc.size();
            for (int j = 0; j < cnt_assoc; j++)
                vector->push_back(Pegasus::CIMInstance(net_assoc[j]));
        }


    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }

    emit doneFetchingData(data);
}

void NetworkProviderPlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    try {
        int cnt = data->size();
        for (int i = 0; i < cnt; i++) {
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance>*) (*data)[i]);

            int network_row_count = m_ui->networks_table->rowCount();
            int interface_row_count;
            int interface_row_sync = -1;

            m_ui->networks_table->insertRow(network_row_count);

            for (unsigned int j = 0; j < vector->size(); j++) {
                if (CIMValue::get_property_value((*vector)[j], "CreationClassName") == "LMI_IPNetworkConnection") {
                    m_ui->networks_table->setItem(
                                network_row_count,
                                0,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "ElementName").c_str())
                                );
                    m_ui->networks_table->setItem(
                                network_row_count,
                                1,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "Description").c_str())
                                );
                    m_ui->networks_table->setItem(
                                network_row_count,
                                2,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "OperatingStatus").c_str())
                                );
                } else if (CIMValue::get_property_value((*vector)[j], "CreationClassName") == "LMI_LANEndpoint") {
                    m_ui->networks_table->setItem(
                                network_row_count,
                                3,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "MACAddress").c_str())
                                );
                } else if (CIMValue::get_property_value((*vector)[j], "CreationClassName") == "LMI_IPProtocolEndpoint") {
                    interface_row_count = m_ui->interfaces_table->rowCount();
                    m_ui->interfaces_table->insertRow(interface_row_count);

                    m_ui->interfaces_table->setItem(
                                interface_row_count,
                                0,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "SystemName").c_str())
                                );
                    m_ui->interfaces_table->setItem(
                                interface_row_count,
                                1,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[0], "ElementName").c_str())
                                );

                    std::string ip = CIMValue::get_property_value((*vector)[j], "IPv4Address");
                    if (ip.empty())
                        ip = CIMValue::get_property_value((*vector)[j], "IPv6Address");

                    m_ui->interfaces_table->setItem(
                                interface_row_count,
                                2,
                                new QTableWidgetItem(ip.c_str())
                                );
                    m_ui->interfaces_table->setItem(
                                interface_row_count,
                                3,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[j], "SubnetMask").c_str())
                                );

                    if (interface_row_sync == -1)
                        continue;

                    std::string expected = CIMValue::get_property_value((*vector)[j], "Name");
                    std::string test = CIMValue::get_property_value((*vector)[interface_row_sync], "Name");

                    int tmp = interface_row_sync;
                    while (CIMValue::get_property_value((*vector)[interface_row_sync], "CreationClassName") == "LMI_NetworkRemoteServiceAccessPoint"
                           && expected[expected.length() - 1] != test[test.length() - 1]) {
                        interface_row_sync++;
                        expected = CIMValue::get_property_value((*vector)[j], "Name");
                        test = CIMValue::get_property_value((*vector)[interface_row_sync], "Name");
                    }

                    m_ui->interfaces_table->setItem(
                                interface_row_count,
                                4,
                                new QTableWidgetItem(CIMValue::get_property_value((*vector)[interface_row_sync], "AccessInfo").c_str())
                                );
                    interface_row_sync = tmp;
                } else if (CIMValue::get_property_value((*vector)[j], "CreationClassName") == "LMI_NetworkRemoteServiceAccessPoint"
                           && CIMValue::get_property_value((*vector)[j], "AccessContext") == "2") {
                    if (interface_row_sync == -1)
                        interface_row_sync = j;
                }
            }
        }

    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    m_changes_enabled = true;
}

Q_EXPORT_PLUGIN2(networkProvider, NetworkProviderPlugin)
