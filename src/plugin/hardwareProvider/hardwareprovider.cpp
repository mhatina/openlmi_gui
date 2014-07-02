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
//    + class LMI_Processor: CIM_Processor
//    - class LMI_ProcessorChip: CIM_Chip
//    - class LMI_ProcessorSystemDevice: CIM_SystemDevice
//    + class LMI_Memory: CIM_Memory
//    + class LMI_PhysicalMemory: CIM_PhysicalMemory
//    - class LMI_MemorySystemDevice: CIM_SystemDevice
//    class LMI_Chassis: CIM_Chassis
//    class LMI_Baseboard: CIM_Card
//    - class LMI_MemorySlot: CIM_Slot
//    class LMI_SystemSlot: CIM_Slot
//    class LMI_PointingDevice: CIM_PointingDevice
//    class LMI_Battery: CIM_Battery
//    class LMI_BatterySystemDevice: CIM_SystemDevice
//    class LMI_PCIDevice: CIM_PCIDevice
//    class LMI_PCIDeviceSystemDevice: CIM_SystemDevice
//    class LMI_PCIBridge: CIM_PCIBridge
//    class LMI_PCIBridgeSystemDevice: CIM_SystemDevice
//    class LMI_DiskDrive: CIM_DiskDrive

//    class LMI_ProcessorCapabilities: CIM_ProcessorCapabilities
//    class LMI_ProcessorElementCapabilities: CIM_ElementCapabilities
//    class LMI_ProcessorCacheMemory: CIM_Memory
//    class LMI_AssociatedProcessorCacheMemory: CIM_AssociatedCacheMemory
//    class LMI_ProcessorChipRealizes: CIM_Realizes
//    class LMI_PhysicalMemoryRealizes: CIM_Realizes
//    class LMI_BaseboardContainer: CIM_Container
//    class LMI_ProcessorChipContainer: CIM_Container
//    class LMI_ChassisComputerSystemPackage: CIM_ComputerSystemPackage
//    class LMI_MemorySlotContainer: CIM_Container
//    class LMI_MemoryPhysicalPackage: CIM_PhysicalPackage
//    class LMI_MemoryPhysicalPackageInConnector: CIM_PackageInConnector
//    class LMI_PhysicalMemoryContainer: CIM_Container
//    class LMI_PortPhysicalConnector: CIM_PhysicalConnector
//    class LMI_PortPhysicalConnectorContainer: CIM_Container
//    class LMI_SystemSlotContainer: CIM_Container
//    class LMI_BatteryPhysicalPackage: CIM_PhysicalPackage
//    class LMI_PhysicalBatteryRealizes: CIM_Realizes
//    class LMI_PhysicalBatteryContainer: CIM_Container
//    class LMI_DiskPhysicalPackage: CIM_PhysicalPackage
//    class LMI_DiskPhysicalPackageContainer: CIM_Container
//    class LMI_DiskDriveRealizes: CIM_Realizes
//    class LMI_DiskDriveSystemDevice: CIM_SystemDevice
//    class LMI_DiskDriveSoftwareIdentity: CIM_SoftwareIdentity
//    class LMI_DiskDriveElementSoftwareIdentity: CIM_ElementSoftwareIdentity
//    class LMI_DiskDriveATAProtocolEndpoint: CIM_ATAProtocolEndpoint
//    class LMI_DiskDriveSAPAvailableForElement: CIM_SAPAvailableForElement
//    class LMI_DiskDriveATAPort: CIM_ATAPort
//    class LMI_DiskDriveDeviceSAPImplementation: CIM_DeviceSAPImplementation
}

void HardwareProviderPlugin::fillTab(std::vector<void *> *data)
{

}

Q_EXPORT_PLUGIN2(hardwareProvider, HardwareProviderPlugin)
