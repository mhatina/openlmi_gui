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

#include "hardware.h"
#include "widgets/labeledlabel.h"
#include "lmiwbem_value.h"
#include "ui_hardware.h"

#include <QScrollArea>
#include <QSpacerItem>
#include <sstream>
#include <vector>

template <typename T>
bool arrayContains(T const array[], int size, T item)
{
    for (int i = 0; i < size; i++) {
        if (array[i] == item) {
            return true;
        }
    }

    return false;
}

template <>
bool arrayContains(const char *const array[], int size, const char *item)
{
    for (int i = 0; i < size; i++) {
        if (strcmp(array[i], item) == 0) {
            return true;
        }
    }

    return false;
}

QTreeWidgetItem *HardwarePlugin::findTopLevelNode(std::string item_name)
{
    QTreeWidgetItem *tmp;
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        tmp = m_ui->tree->topLevelItem(i);
        if (tmp != NULL && tmp->text(0).toStdString() == item_name) {
            return tmp;
        }
    }

    return NULL;
}

QTreeWidgetItem *HardwarePlugin::topLevelNode(std::string item_name)
{
    QTreeWidgetItem *node;
    if (!(node = findTopLevelNode(item_name))) {
        node = new QTreeWidgetItem(m_ui->tree);
        node->setText(0, item_name.c_str());
        node->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
        m_ui->tree->sortByColumn(
            0,
            Qt::AscendingOrder
        );
    }

    return node;
}

std::string HardwarePlugin::decodeValues(Pegasus::CIMProperty property)
{
    Pegasus::CIMValue value = property.getValue();
    if (property.getName().equal(Pegasus::CIMName("Flags"))) {
        Pegasus::Array<Pegasus::Uint16> raw_array;
        std::stringstream ss;
        value.get(raw_array);
        const Pegasus::Uint32 cnt = value.getArraySize();
        for (Pegasus::Uint32 i = 0; i < cnt; ++i) {
            const Pegasus::Uint16 &raw_value = raw_array[i];
            ss << m_values[CIMValue::to_std_string(raw_value)];
            if (i < cnt - 1) {
                ss << ", ";
            }
        }
        return ss.str();
    }

    return "";
}

void HardwarePlugin::clearComponentInfo()
{
    QObjectList list = m_ui->device_box->children();

    for (int i = list.size() - 1; i >= 0; i--) {
        m_ui->device_box->layout()->removeWidget(qobject_cast<QWidget *>(list[i]));
        delete list[i];
    }
    m_ui->device_box->setLayout(new QFormLayout());
}

void HardwarePlugin::fillBattery(Pegasus::CIMInstance battery)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Battery");

    layout->addWidget(new LabeledLabel("Capacity", CIMValue::get_property_value(battery,
                                       "DesignCapacity") + " mWatt * hour"));
    layout->addWidget(new LabeledLabel("Voltage", CIMValue::get_property_value(battery, "DesignVoltage") + " mVolt"));
    layout->addWidget(new LabeledLabel("Chemistry", CIMValue::get_property_value(battery, "Chemistry")));

    setAlignment();
}

void HardwarePlugin::fillChassis(Pegasus::CIMInstance chassis)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Chassis");

    layout->addWidget(new LabeledLabel("Name", CIMValue::get_property_value(chassis, "Name")));
    layout->addWidget(new LabeledLabel("Manufacturer", CIMValue::get_property_value(chassis, "Manufacturer")));
    layout->addWidget(new LabeledLabel("Model", CIMValue::get_property_value(chassis, "Model")));
    layout->addWidget(new LabeledLabel("Package Type", CIMValue::get_property_value(chassis, "PackageType")));
    layout->addWidget(new LabeledLabel("Product Name", CIMValue::get_property_value(chassis, "ProductName")));
    layout->addWidget(new LabeledLabel("Serial Number", CIMValue::get_property_value(chassis, "SerialNumber")));
    layout->addWidget(new LabeledLabel("UUID", CIMValue::get_property_value(chassis, "UUID")));

    setAlignment();
}

void HardwarePlugin::fillDisk(std::vector<Pegasus::CIMInstance> disk)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Disk drive");

    layout->addWidget(new LabeledLabel("Name", CIMValue::get_property_value(disk[0], "Name")));
    layout->addWidget(new LabeledLabel("Type", CIMValue::get_property_value(disk[0], "DiskType")));
    layout->addWidget(new LabeledLabel("Model", CIMValue::get_property_value(disk[1], "Model")));
    layout->addWidget(new LabeledLabel("Serial number", CIMValue::get_property_value(disk[1], "SerialNumber")));
    layout->addWidget(new LabeledLabel("ID", CIMValue::get_property_value(disk[0], "DeviceID")));
    std::string capacity = CIMValue::get_property_value(disk[0], "Capacity");
    layout->addWidget(new LabeledLabel("Capacity", CIMValue::convert_values(capacity, "B")));
    layout->addWidget(new LabeledLabel("Interconnect", CIMValue::get_property_value(disk[0], "InterconnectType")));
    std::string speed = CIMValue::get_property_value(disk[0], "InterconnectSpeed");
    layout->addWidget(new LabeledLabel("Interconnect speed", CIMValue::convert_values(speed, "b/s")));
    ushort degree = 0x00b0;
    std::string degree_str = QString::fromUtf16(&degree).toStdString().substr(0, 1) + " C";
    layout->addWidget(new LabeledLabel("Temperature", CIMValue::get_property_value(disk[0], "Temperature") + degree_str));
    layout->addWidget(new LabeledLabel("RPM", CIMValue::get_property_value(disk[0], "RPM")));

    setAlignment();
}

void HardwarePlugin::fillMemory(std::vector<Pegasus::CIMInstance> memory)
{
    Pegasus::CIMInstance mem = memory[1];
    Pegasus::CIMInstance phys_mem = memory[0];
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Memory");

    layout->addWidget(new LabeledLabel("Access", CIMValue::get_property_value(mem, "Access")));
    layout->addWidget(new LabeledLabel("Capacity", CIMValue::convert_values(CIMValue::get_property_value(phys_mem, "Capacity"), "B")));
    layout->addWidget(new LabeledLabel("Form factor", CIMValue::get_property_value(phys_mem, "FormFactor")));
    layout->addWidget(new LabeledLabel("Memory type", CIMValue::get_property_value(phys_mem, "MemoryType")));
    layout->addWidget(new LabeledLabel("Total width", CIMValue::convert_values(CIMValue::get_property_value(phys_mem, "TotalWidth"), "b")));
    layout->addWidget(new LabeledLabel("Page size", CIMValue::convert_values(CIMValue::get_property_value(mem, "StandardMemoryPageSize"), "b")));

    std::string tmp = CIMValue::get_property_value(phys_mem, "ConfiguredMemoryClockSpeed") + " MHz";
    layout->addWidget(new LabeledLabel("Clock speed", tmp));
    layout->addWidget(new LabeledLabel("Serial number", CIMValue::get_property_value(phys_mem, "SerialNumber")));
    layout->addWidget(new LabeledLabel("Part number", CIMValue::get_property_value(phys_mem, "PartNumber")));
    layout->addWidget(new LabeledLabel("Manufacturer", CIMValue::get_property_value(phys_mem, "Manufacturer")));
    layout->addWidget(new LabeledLabel("Bank", CIMValue::get_property_value(phys_mem, "BankLabel")));

    setAlignment();
}

void HardwarePlugin::fillPCI(Pegasus::CIMInstance pci)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("PCI");

    std::string ttmp = CIMValue::get_property_value(pci, "BusNumber");
    layout->addWidget(new LabeledLabel("Bus Number", ttmp));
    layout->addWidget(new LabeledLabel("Device Number", CIMValue::get_property_value(pci, "DeviceNumber")));
    layout->addWidget(new LabeledLabel("Function Number", CIMValue::get_property_value(pci, "FunctionNumber")));
    layout->addWidget(new LabeledLabel("PCI Device ID", CIMValue::get_property_value(pci, "DeviceID")));
    layout->addWidget(new LabeledLabel("PCI Device Name", CIMValue::get_property_value(pci, "Name")));
    layout->addWidget(new LabeledLabel("Vendor ID", CIMValue::get_property_value(pci, "VendorID")));
    layout->addWidget(new LabeledLabel("Vendor Name", CIMValue::get_property_value(pci, "VendorName")));
    layout->addWidget(new LabeledLabel("Subsystem ID", CIMValue::get_property_value(pci, "SubsystemID")));
    layout->addWidget(new LabeledLabel("Subsystem Name", CIMValue::get_property_value(pci, "SubsystemName")));
    layout->addWidget(new LabeledLabel("Subsystem Vendor ID", CIMValue::get_property_value(pci, "SubsystemVendorID")));
    layout->addWidget(new LabeledLabel("Subsystem Vendor Name", CIMValue::get_property_value(pci, "SubsystemVendorName")));
    layout->addWidget(new LabeledLabel("Revision ID", CIMValue::get_property_value(pci, "RevisionID")));

    std::string tmp = CIMValue::get_property_value(pci, "BaseAddress");
    layout->addWidget(new LabeledLabel("Base Address", tmp.empty() ? CIMValue::get_property_value(pci,
                                       "BaseAddress64") : tmp));
    layout->addWidget(new LabeledLabel("Cache Line Size", CIMValue::get_property_value(pci, "CacheLineSize")));
    layout->addWidget(new LabeledLabel("Capabilities", CIMValue::get_property_value(pci, "Capabilities")));
    layout->addWidget(new LabeledLabel("Device Select Timing", CIMValue::get_property_value(pci, "DeviceSelectTiming")));
    layout->addWidget(new LabeledLabel("Interrupt Pin", CIMValue::get_property_value(pci, "InterruptPin")));
    layout->addWidget(new LabeledLabel("Latency Timer", CIMValue::get_property_value(pci, "LatencyTimer")));
    layout->addWidget(new LabeledLabel("Expansion ROM Base Address", CIMValue::get_property_value(pci,
                                       "ExpansionROMBaseAddress")));

    if (CIMValue::get_property_value(pci, "CreationClassName") != "LMI_PCIBridge") {
        setAlignment();
        return;
    }

    layout->addWidget(new LabeledLabel("Bridge Type", CIMValue::get_property_value(pci, "BridgeType")));
    layout->addWidget(new LabeledLabel("Primary Bus Number", CIMValue::get_property_value(pci, "PrimaryBusNumber")));

    // BUG SecondaryBusNumber
    layout->addWidget(new LabeledLabel("Secondary Bus Number", CIMValue::get_property_value(pci, "SecondayBusNumber")));
    layout->addWidget(new LabeledLabel("Subordinate Bus Number", CIMValue::get_property_value(pci,
                                       "SubordinateBusNumber")));
    layout->addWidget(new LabeledLabel("Secondary Latency Timer", CIMValue::get_property_value(pci,
                                       "SecondaryLatencyTimer")));
    layout->addWidget(new LabeledLabel("IO Base", CIMValue::get_property_value(pci, "IOBase")));
    layout->addWidget(new LabeledLabel("IO Limit", CIMValue::get_property_value(pci, "IOLimit")));
    layout->addWidget(new LabeledLabel("Memory Base", CIMValue::get_property_value(pci, "MemoryBase")));
    layout->addWidget(new LabeledLabel("Memory Limit", CIMValue::get_property_value(pci, "MemoryLimit")));
    layout->addWidget(new LabeledLabel("Prefetch Memory Base", CIMValue::get_property_value(pci, "PrefetchMemoryBase")));
    layout->addWidget(new LabeledLabel("Prefetch Memory Limit", CIMValue::get_property_value(pci, "PrefetchMemoryLimit")));

    setAlignment();
}

void HardwarePlugin::fillPort(Pegasus::CIMInstance port)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Port");

    layout->addWidget(new LabeledLabel("Name", CIMValue::get_property_value(port, "Name")));
    layout->addWidget(new LabeledLabel("Description", CIMValue::get_property_value(port, "ConnectorDescription")));
    layout->addWidget(new LabeledLabel("Gender", CIMValue::get_property_value(port, "ConnectorGender")));
    layout->addWidget(new LabeledLabel("Layout", CIMValue::get_property_value(port, "ConnectorLayout")));

    setAlignment();
}

void HardwarePlugin::fillProcessor(std::vector<Pegasus::CIMInstance> processor)
{
    Pegasus::CIMInstance proc = processor[0];
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("CPU");

    layout->addWidget(new LabeledLabel("Device ID", CIMValue::get_property_value(proc, "DeviceID")));
    layout->addWidget(new LabeledLabel("Name", CIMValue::get_property_value(proc, "ElementName")));
    std::string arch = CIMValue::get_property_value(proc, "Architecture");
    layout->addWidget(new LabeledLabel("Architecture", arch));
    std::string tmp = CIMValue::get_property_value(proc, "MaxClockSpeed") + " MHz";
    layout->addWidget(new LabeledLabel("Maximum clock speed", tmp));
    Pegasus::CIMInstance proc_capabilities = processor[1];
    layout->addWidget(new LabeledLabel("Cores", CIMValue::get_property_value(proc_capabilities, "NumberOfProcessorCores")));
    layout->addWidget(new LabeledLabel("Hardware threads", CIMValue::get_property_value(proc_capabilities,
                                       "NumberOfHardwareThreads")));

    layout->addWidget(new LabeledLabel("Address width", CIMValue::convert_values(CIMValue::get_property_value(proc, "AddressWidth"), "b")));
    layout->addWidget(new LabeledLabel("Data width", CIMValue::convert_values(CIMValue::get_property_value(proc, "DataWidth"), "b")));

    tmp = CIMValue::get_property_value(proc, "CurrentClockSpeed") + " MHz";
    layout->addWidget(new LabeledLabel("Current clock speed", tmp));

    if (arch.find("x86") != std::string::npos) {
        Pegasus::Uint32 prop_ind = proc.findProperty("Flags");
        LabeledLabel *label = new LabeledLabel("Flags", decodeValues(proc.getProperty(prop_ind)));
        label->setVerticalAlignment(Qt::AlignTop);
        QScrollArea *area = new QScrollArea();

        QFormLayout *form = new QFormLayout();
        form->setMargin(0);
        area->setLayout(form);
        area->layout()->addWidget(label);
        layout->addWidget(area);
    }


    QGroupBox *cache_box = new QGroupBox("Cache");
    cache_box->setLayout(new QFormLayout());
    cache_box->setObjectName("cache_box");
    layout->addWidget(cache_box);
    for (unsigned int i = 2; i < processor.size(); i++) {
        std::string name = CIMValue::get_property_value(processor[i], "Name");

        Pegasus::Uint32 propIndex = processor[i].findProperty(
                                        Pegasus::CIMName("NumberOfBlocks"));
        Pegasus::CIMProperty prop = processor[i].getProperty(propIndex);
        Pegasus::CIMValue value = prop.getValue();

        std::stringstream ss;
        ss << value.toString();
        Pegasus::Uint64 blocks;
        ss >> blocks;

        propIndex = processor[i].findProperty(Pegasus::CIMName("BlockSize"));
        prop = processor[i].getProperty(propIndex);
        value = prop.getValue();

        ss.str("");
        ss.clear();
        ss << value.toString();
        Pegasus::Uint64 size;
        ss >> size;

        ss.str("");
        ss.clear();
        ss << (blocks * size);

        cache_box->layout()->addWidget(new LabeledLabel(name, CIMValue::convert_values(ss.str(), "B")));
    }

    setAlignment();
}

void HardwarePlugin::setAlignment()
{
    int max = 0;
    QObjectList children = m_ui->device_box->children();
    for (int i = 0; i < children.size(); i++) {
        if (children.at(i)->objectName() != "labeledLabel") {
            continue;
        }

        if (((LabeledLabel *) children[i])->getLabelWidth() > max) {
            max = ((LabeledLabel *) children[i])->getLabelWidth();
        }
    }

    for (int i = 0; i < children.size(); i++) {
        if (children.at(i)->objectName() != "labeledLabel") {
            continue;
        }

        ((LabeledLabel *) children[i])->setAlignment(max);
    }
}

HardwarePlugin::HardwarePlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::HardwarePlugin)
{
    m_ui->setupUi(this);
    m_ui->filter_box->hide();
    setPluginEnabled(false);

    int cnt = sizeof(valueMap) / sizeof(valueMap[0]);
    for (int i = 0; i < cnt; i++) {
        m_values[valueMap[i]] = values[i];
    }

    connect(
        m_ui->tree,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(showComponent()));
}

HardwarePlugin::~HardwarePlugin()
{
    delete m_ui;
}

std::string HardwarePlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string HardwarePlugin::getLabel()
{
    return "Hardware";
}

std::string HardwarePlugin::getRefreshInfo()
{
    int cnt = m_battery.size() + m_chassis.size() +
              m_pci_bridge.size() + m_pci_device.size() +
              m_port.size() + m_memory.size() +
              m_processor.size();
    std::stringstream ss;
    ss << getLabel() << ": " << cnt << " component(s) shown";
    return ss.str();
}

void HardwarePlugin::getData(std::vector<void *> *data)
{
    Pegasus::Array<Pegasus::CIMInstance> array;
    int size = 0;

    try {
        // processor
        array = enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_Processor"),
                    true,       // deep inheritance
                    false,      // local only
                    true,       // include qualifiers
                    false       // include class origin
                );

        unsigned int cnt = array.size();
        for (unsigned int i = 0; i < cnt; i++, size++) {
            data->push_back(new std::vector<Pegasus::CIMInstance>());
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance>
                    *) (*data)[size]);
            vector->push_back(array[i]);

            Pegasus::Array<Pegasus::CIMObject> proc_capab =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    array[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_ProcessorCapabilities")
                );
            vector->push_back(Pegasus::CIMInstance(proc_capab[0]));

            Pegasus::Array<Pegasus::CIMObject> cache =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    array[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_ProcessorCacheMemory")
                );
            for (unsigned int j = 0; j < cache.size(); j++) {
                vector->push_back(Pegasus::CIMInstance(cache[j]));
            }
        }

        // memory
        array = enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_PhysicalMemory"),
                    true,       // deep inheritance
                    false,      // local only
                    true,       // include qualifiers
                    false       // include class origin
                );

        cnt = array.size();
        for (unsigned int i = 0; i < cnt; i++, size++) {
            data->push_back(new std::vector<Pegasus::CIMInstance>());
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance>
                    *) (*data)[size]);
            vector->push_back(array[i]);

            Pegasus::Array<Pegasus::CIMObject> memory =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    array[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_Memory")
                );
            vector->push_back(Pegasus::CIMInstance(memory[0]));
        }

        // disk drive
        array = enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_DiskDrive"),
                    true,       // deep inheritance
                    false,      // local only
                    true,       // include qualifiers
                    false       // include class origin
                );

        cnt = array.size();
        for (unsigned int i = 0; i < cnt; i++, size++) {
            data->push_back(new std::vector<Pegasus::CIMInstance>());
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance>
                    *) (*data)[size]);
            vector->push_back(array[i]);

            Pegasus::Array<Pegasus::CIMObject> disk =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    array[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName("LMI_DiskPhysicalPackage")
                );
            vector->push_back(Pegasus::CIMInstance(disk[0]));
        }

        const char *devices[] = {
            "LMI_PCIDevice",
            "LMI_PCIBridge",
            "LMI_Chassis",
            "LMI_PortPhysicalConnector",
            "LMI_Battery"
        };
        int device_cnt = sizeof(devices) / sizeof(devices[0]);
        for (int j = 0; j < device_cnt; j++) {

            array = enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName(devices[j]),
                        true,       // deep inheritance
                        false,      // local only
                        true,       // include qualifiers
                        false       // include class origin
                    );

            std::vector<Pegasus::CIMInstance> *vector = new
            std::vector<Pegasus::CIMInstance>();
            data->push_back(vector);

            cnt = array.size();
            for (unsigned int i = 0; i < cnt; i++, size++) {
                vector->push_back(array[i]);
            }
        }
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    emit doneFetchingData(data);
}

void HardwarePlugin::clear()
{
    m_changes_enabled = false;
    m_ui->tree->clear();
    clearComponentInfo();
    m_changes_enabled = true;
}

void HardwarePlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    try {
        std::vector<Pegasus::CIMInstance> *vector;
        unsigned int pos = 0;
        bool pci_device = true;
        while (pos < data->size()) {
            vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[pos++]);
            if (vector->empty()) {
                continue;
            }
            if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Processor"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                QTreeWidgetItem *item = new QTreeWidgetItem();
                std::string name = CIMValue::get_property_value(tmp[0], "ElementName");
                item->setText(0, name.c_str());
                item->setToolTip(0, name.c_str());
                topLevelNode("Processor")->addChild(item);

                m_processor.push_back(tmp);
            } else if ((*vector)[0].getClassName().equal(
                           Pegasus::CIMName("LMI_PhysicalMemory"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                QTreeWidgetItem *item = new QTreeWidgetItem();
                std::string name = CIMValue::get_property_value(tmp[0],
                                   "ElementName") + " " + CIMValue::get_property_value(tmp[0], "PartNumber");
                item->setText(0, name.c_str());
                item->setToolTip(0, name.c_str());
                topLevelNode("Memory")->addChild(item);

                m_memory.push_back(tmp);
            } else if ((pci_device = (*vector)[0].getClassName().equal(
                                         Pegasus::CIMName("LMI_PCIDevice")))
                       || (*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_PCIBridge"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = CIMValue::get_property_value(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    Pegasus::Uint32 prop_ind = tmp[i].findProperty("ClassCode");
                    Pegasus::CIMProperty prop = tmp[i].getProperty(prop_ind);
                    Pegasus::Uint8 value;
                    prop.getValue().get(value);
                    switch (value) {
                    case 1:
                    case 2:
                    case 3: {
                        std::string prop_name = CIMValue::decode_values(prop);
                        prop_name += " Controller";
                        topLevelNode(prop_name)->addChild(item);
                        break;
                    }
                    default:
                        topLevelNode(CIMValue::decode_values(prop))->addChild(item);
                    }
                }
                if (pci_device) {
                    m_pci_device = tmp;
                } else {
                    m_pci_bridge = tmp;
                }
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Chassis"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = CIMValue::get_property_value(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Chassis")->addChild(item);
                }
                m_chassis = tmp;
            } else if ((*vector)[0].getClassName().equal(
                           Pegasus::CIMName("LMI_PortPhysicalConnector"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = CIMValue::get_property_value(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Physical port")->addChild(item);
                }
                m_port = tmp;
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Battery"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = CIMValue::get_property_value(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Battery")->addChild(item);
                }
                m_battery = tmp;
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_DiskDrive"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *)
                                                        (*data)[pos - 1]);

                QTreeWidgetItem *item = new QTreeWidgetItem();
                std::string name = CIMValue::get_property_value(tmp[0], "Name") +
                        " " + CIMValue::get_property_value(tmp[1], "SerialNumber");
                item->setText(0, name.c_str());
                item->setToolTip(0, name.c_str());

                topLevelNode("Disk Drive")->addChild(item);

                m_disk_drive.push_back(tmp);
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    m_changes_enabled = true;

    for (unsigned int i = 0; i < data->size(); i++) {
        delete ((std::vector<Pegasus::CIMInstance> *) (*data)[i]);
    }
}

void HardwarePlugin::showComponent()
{
    if (m_ui->tree->selectedItems().empty()) {
        return;
    }

    QTreeWidgetItem *item = m_ui->tree->selectedItems()[0];
    std::string parent = item->parent()->text(0).toStdString();
    std::string name = item->text(0).toStdString();

    if (parent.find("Controller") != std::string::npos) {
        parent = parent.substr(0, parent.rfind(" "));
    }

    clearComponentInfo();

    if (parent == "Processor") {
        for (unsigned int i = 0; i < m_processor.size(); i++) {
            std::vector<Pegasus::CIMInstance> proc = m_processor[i];
            if (name != CIMValue::get_property_value(proc[0], "ElementName")) {
                continue;
            }

            fillProcessor(proc);
            break;
        }
    } else if (parent == "Memory") {
        name = name.substr(name.rfind(" ") + 1);
        for (unsigned int i = 0; i < m_memory.size(); i++) {
            std::vector<Pegasus::CIMInstance> mem = m_memory[i];

            if (name != CIMValue::get_property_value(mem[0], "PartNumber")) {
                continue;
            }

            fillMemory(mem);
            break;
        }
    } else if (parent == "Battery") {
        for (unsigned int i = 0; i < m_battery.size(); i++) {
            Pegasus::CIMInstance batt = m_battery[i];

            if (name != CIMValue::get_property_value(batt, "Name")) {
                continue;
            }

            fillBattery(batt);
            break;
        }
    } else if (arrayContains<const char *>(class_code_values,
                                           sizeof(class_code_values) / sizeof(class_code_values[0]), parent.c_str())) {
        for (unsigned int i = 0; i < m_pci_device.size(); i++) {
            Pegasus::CIMInstance pci = m_pci_device[i];

            if (name != CIMValue::get_property_value(pci, "Name")) {
                continue;
            }

            fillPCI(pci);
            return;
        }
        for (unsigned int i = 0; i < m_pci_bridge.size(); i++) {
            Pegasus::CIMInstance bridge = m_pci_bridge[i];

            if (name != CIMValue::get_property_value(bridge, "Name")) {
                continue;
            }

            fillPCI(bridge);
            break;
        }
    } else if (parent == "Chassis") {
        for (unsigned int i = 0; i < m_chassis.size(); i++) {
            Pegasus::CIMInstance chassis = m_chassis[i];

            if (name != CIMValue::get_property_value(chassis, "Name")) {
                continue;
            }

            fillChassis(chassis);
            break;
        }
    } else if (parent == "Physical port") {
        for (unsigned int i = 0; i < m_port.size(); i++) {
            Pegasus::CIMInstance port = m_port[i];

            if (name != CIMValue::get_property_value(port, "Name")) {
                continue;
            }

            fillPort(port);
            break;
        }
    } else if (parent == "Disk Drive") {
        name = name.substr(name.rfind(" ") + 1);
        for (unsigned int i = 0; i < m_disk_drive.size(); i++) {
            std::vector<Pegasus::CIMInstance> disk = m_disk_drive[i];

            if (name != CIMValue::get_property_value(disk[1], "SerialNumber")) {
                continue;
            }

            fillDisk(disk);
            break;
        }
    }
}

Q_EXPORT_PLUGIN2(hardware, HardwarePlugin)
