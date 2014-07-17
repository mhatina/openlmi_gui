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
#include "labeledlabel.h"
#include "lmiwbem_value.h"
#include "ui_hardwareprovider.h"

#include <QScrollArea>
#include <QSpacerItem>
#include <sstream>
#include <vector>

template <typename T>
bool arrayContains(T const array[], int size, T item)
{
    for (int i = 0; i < size; i++) {
        if (array[i] == item)
            return true;
    }

    return false;
}

template <>
bool arrayContains(const char* const array[], int size, const char* item)
{
    for (int i = 0; i < size; i++) {
        if (strcmp(array[i], item) == 0)
            return true;
    }

    return false;
}

QTreeWidgetItem* HardwareProviderPlugin::findTopLevelNode(std::string item_name)
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

QTreeWidgetItem* HardwareProviderPlugin::topLevelNode(std::string item_name)
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

std::string HardwareProviderPlugin::convertCapacity(std::string capacity, bool bit)
{
    std::stringstream ss;
    Pegasus::Uint64 c;
    ss << capacity;
    ss >> c;
    Pegasus::Uint64 tmp = c;
    int i = 0;

    while ((tmp = c / 1024) > 0) {
        c = tmp;
        i++;
    }

    ss.str("");
    ss.clear();

    ss << c;
    switch (i) {
    case 0:
        ss << " " << (bit ? "b" : "B");
        break;
    case 1:
        ss << " k" << (bit ? "b" : "B");
        break;
    case 2:
        ss << " M" << (bit ? "b" : "B");
        break;
    case 3:
        ss << " G" << (bit ? "b" : "B");
        break;
    case 4:
        ss << " T" << (bit ? "b" : "B");
        break;
    default:
        ss << " ?" << (bit ? "b" : "B");
        break;
    }

    return ss.str();
}

std::string HardwareProviderPlugin::decodeValues(Pegasus::CIMProperty property)
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
            if (i < cnt - 1)
                ss << ", ";
        }
        return ss.str();
    } else if (property.getName().equal(Pegasus::CIMName("MemoryType"))) {
        return memory_type_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("FormFactor"))) {
        return form_factor_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("Access"))) {
        return access_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("ClassCode"))) {
        return class_code_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("Chemistry"))) {
        return chemistry_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("ConnectorGender"))) {
        return connector_gender_values[atoi(value.toString().getCString())];
    } else if (property.getName().equal(Pegasus::CIMName("ConnectorLayout"))) {
        return connector_layout_values[atoi(value.toString().getCString())];
    }

    return "";
}

void HardwareProviderPlugin::fillBattery(Pegasus::CIMInstance battery)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Battery");

    layout->addWidget(new LabeledLabel("Capacity", getPropertyOfInstance(battery, "DesignCapacity") + " mWatt * hour"));
    layout->addWidget(new LabeledLabel("Voltage", getPropertyOfInstance(battery, "DesignVoltage") + " mVolt"));

    Pegasus::Uint32 prop_ind = battery.findProperty(Pegasus::CIMName("Chemistry"));
    layout->addWidget(new LabeledLabel("Chemistry", decodeValues(battery.getProperty(prop_ind))));

    setAlignment();
}

void HardwareProviderPlugin::fillChassis(Pegasus::CIMInstance chassis)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Chassis");

    layout->addWidget(new LabeledLabel("Name", getPropertyOfInstance(chassis, "Name")));
    layout->addWidget(new LabeledLabel("Manufacturer", getPropertyOfInstance(chassis, "Manufacturer")));
    layout->addWidget(new LabeledLabel("Model", getPropertyOfInstance(chassis, "Model")));
    layout->addWidget(new LabeledLabel("Package Type", getPropertyOfInstance(chassis, "PackageType")));
    layout->addWidget(new LabeledLabel("Product Name", getPropertyOfInstance(chassis, "ProductName")));
    layout->addWidget(new LabeledLabel("Serial Number", getPropertyOfInstance(chassis, "SerialNumber")));
    layout->addWidget(new LabeledLabel("UUID", getPropertyOfInstance(chassis, "UUID")));

    setAlignment();
}

void HardwareProviderPlugin::fillMemory(std::vector<Pegasus::CIMInstance> memory)
{
    Pegasus::CIMInstance mem = memory[1];
    Pegasus::CIMInstance phys_mem = memory[0];
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Memory");

    Pegasus::Uint32 prop_ind = mem.findProperty(Pegasus::CIMName("Access"));
    layout->addWidget(new LabeledLabel("Access", decodeValues(mem.getProperty(prop_ind))));

    layout->addWidget(new LabeledLabel("Capacity", convertCapacity(getPropertyOfInstance(phys_mem, "Capacity"))));

    prop_ind = phys_mem.findProperty(Pegasus::CIMName("FormFactor"));
    layout->addWidget(new LabeledLabel("Form factor", decodeValues(phys_mem.getProperty(prop_ind))));

    prop_ind = phys_mem.findProperty(Pegasus::CIMName("MemoryType"));
    layout->addWidget(new LabeledLabel("Memory type", decodeValues(phys_mem.getProperty(prop_ind))));

    layout->addWidget(new LabeledLabel("Total width", convertCapacity(getPropertyOfInstance(phys_mem, "TotalWidth"))));

    layout->addWidget(new LabeledLabel("Page size", convertCapacity(getPropertyOfInstance(mem, "StandardMemoryPageSize"), true)));

    std::string tmp = getPropertyOfInstance(phys_mem, "ConfiguredMemoryClockSpeed") + " MHz";
    layout->addWidget(new LabeledLabel("Clock speed", tmp));

    layout->addWidget(new LabeledLabel("Serial number", getPropertyOfInstance(phys_mem, "SerialNumber")));
    layout->addWidget(new LabeledLabel("Part number", getPropertyOfInstance(phys_mem, "PartNumber")));
    layout->addWidget(new LabeledLabel("Manufacturer", getPropertyOfInstance(phys_mem, "Manufacturer")));
    layout->addWidget(new LabeledLabel("Bank", getPropertyOfInstance(phys_mem, "BankLabel")));

    setAlignment();
}

void HardwareProviderPlugin::fillPCI(Pegasus::CIMInstance pci)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("PCI");

    std::string ttmp = getPropertyOfInstance(pci, "BusNumber");
    layout->addWidget(new LabeledLabel("Bus Number", ttmp));
    layout->addWidget(new LabeledLabel("Device Number", getPropertyOfInstance(pci, "DeviceNumber")));
    layout->addWidget(new LabeledLabel("Function Number", getPropertyOfInstance(pci, "FunctionNumber")));
    layout->addWidget(new LabeledLabel("PCI Device ID", getPropertyOfInstance(pci, "DeviceID")));
    layout->addWidget(new LabeledLabel("PCI Device Name", getPropertyOfInstance(pci, "Name")));
    layout->addWidget(new LabeledLabel("Vendor ID", getPropertyOfInstance(pci, "VendorID")));
    layout->addWidget(new LabeledLabel("Vendor Name", getPropertyOfInstance(pci, "VendorName")));
    layout->addWidget(new LabeledLabel("Subsystem ID", getPropertyOfInstance(pci, "SubsystemID")));
    layout->addWidget(new LabeledLabel("Subsystem Name", getPropertyOfInstance(pci, "SubsystemName")));
    layout->addWidget(new LabeledLabel("Subsystem Vendor ID", getPropertyOfInstance(pci, "SubsystemVendorID")));
    layout->addWidget(new LabeledLabel("Subsystem Vendor Name", getPropertyOfInstance(pci, "SubsystemVendorName")));
    layout->addWidget(new LabeledLabel("Revision ID", getPropertyOfInstance(pci, "RevisionID")));

    std::string tmp = getPropertyOfInstance(pci, "BaseAddress");
    layout->addWidget(new LabeledLabel("Base Address", tmp.empty() ? getPropertyOfInstance(pci, "BaseAddress64") : tmp));
    layout->addWidget(new LabeledLabel("Cache Line Size", getPropertyOfInstance(pci, "CacheLineSize")));
    layout->addWidget(new LabeledLabel("Capabilities", getPropertyOfInstance(pci, "Capabilities")));
    layout->addWidget(new LabeledLabel("Device Select Timing", getPropertyOfInstance(pci, "DeviceSelectTiming")));
    layout->addWidget(new LabeledLabel("Interrupt Pin", getPropertyOfInstance(pci, "InterruptPin")));
    layout->addWidget(new LabeledLabel("Latency Timer", getPropertyOfInstance(pci, "LatencyTimer")));
    layout->addWidget(new LabeledLabel("Expansion ROM Base Address", getPropertyOfInstance(pci, "ExpansionROMBaseAddress")));

    if (getPropertyOfInstance(pci, "CreationClassName") != "LMI_PCIBridge") {
        setAlignment();
        return;
    }

    layout->addWidget(new LabeledLabel("Bridge Type", getPropertyOfInstance(pci, "BridgeType")));
    layout->addWidget(new LabeledLabel("Primary Bus Number", getPropertyOfInstance(pci, "PrimaryBusNumber")));

    // BUG SecondaryBusNumber
    layout->addWidget(new LabeledLabel("Secondary Bus Number", getPropertyOfInstance(pci, "SecondayBusNumber")));
    layout->addWidget(new LabeledLabel("Subordinate Bus Number", getPropertyOfInstance(pci, "SubordinateBusNumber")));
    layout->addWidget(new LabeledLabel("Secondary Latency Timer", getPropertyOfInstance(pci, "SecondaryLatencyTimer")));
    layout->addWidget(new LabeledLabel("IO Base", getPropertyOfInstance(pci, "IOBase")));
    layout->addWidget(new LabeledLabel("IO Limit", getPropertyOfInstance(pci, "IOLimit")));
    layout->addWidget(new LabeledLabel("Memory Base", getPropertyOfInstance(pci, "MemoryBase")));
    layout->addWidget(new LabeledLabel("Memory Limit", getPropertyOfInstance(pci, "MemoryLimit")));
    layout->addWidget(new LabeledLabel("Prefetch Memory Base", getPropertyOfInstance(pci, "PrefetchMemoryBase")));
    layout->addWidget(new LabeledLabel("Prefetch Memory Limit", getPropertyOfInstance(pci, "PrefetchMemoryLimit")));

    setAlignment();
}

void HardwareProviderPlugin::fillPort(Pegasus::CIMInstance port)
{
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("Port");

    layout->addWidget(new LabeledLabel("Name", getPropertyOfInstance(port, "Name")));
    layout->addWidget(new LabeledLabel("Description", getPropertyOfInstance(port, "ConnectorDescription")));
    Pegasus::Uint32 prop_ind = port.findProperty("ConnectorGender");
    layout->addWidget(new LabeledLabel("Gender", decodeValues(port.getProperty(prop_ind))));
    prop_ind = port.findProperty("ConnectorLayout");
    layout->addWidget(new LabeledLabel("Layout", decodeValues(port.getProperty(prop_ind))));

    setAlignment();
}

void HardwareProviderPlugin::fillProcessor(std::vector<Pegasus::CIMInstance> processor)
{    
    Pegasus::CIMInstance proc = processor[0];
    QLayout *layout = m_ui->device_box->layout();
    m_ui->device_box->setTitle("CPU");

    layout->addWidget(new LabeledLabel("Device ID", getPropertyOfInstance(proc, "DeviceID")));
    layout->addWidget(new LabeledLabel("Name", getPropertyOfInstance(proc, "ElementName")));
    std::string arch = getPropertyOfInstance(proc, "Architecture");
    layout->addWidget(new LabeledLabel("Architecture", arch));
    std::string tmp = getPropertyOfInstance(proc, "MaxClockSpeed") + " MHz";
    layout->addWidget(new LabeledLabel("Maximum clock speed", tmp));
    Pegasus::CIMInstance proc_capabilities = processor[1];
    layout->addWidget(new LabeledLabel("Cores", getPropertyOfInstance(proc_capabilities, "NumberOfProcessorCores")));
    layout->addWidget(new LabeledLabel("Hardware threads", getPropertyOfInstance(proc_capabilities, "NumberOfHardwareThreads")));

    layout->addWidget(new LabeledLabel("Address width", convertCapacity(getPropertyOfInstance(proc, "AddressWidth"), true)));
    layout->addWidget(new LabeledLabel("Data width", convertCapacity(getPropertyOfInstance(proc, "DataWidth"), true)));

    tmp = getPropertyOfInstance(proc, "CurrentClockSpeed") + " MHz";
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
        std::string name = getPropertyOfInstance(processor[i], "Name");

        Pegasus::Uint32 propIndex = processor[i].findProperty(Pegasus::CIMName("NumberOfBlocks"));
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

        cache_box->layout()->addWidget(new LabeledLabel(name, convertCapacity(ss.str())));
    }

    setAlignment();
}

void HardwareProviderPlugin::setAlignment()
{
    int max = 0;
    QObjectList children = m_ui->device_box->children();
    for (int i = 0; i < children.size(); i++) {
        if (children.at(i)->objectName() != "labeledLabel")
            continue;

        if (((LabeledLabel*) children[i])->getLabelWidth() > max)
            max = ((LabeledLabel*) children[i])->getLabelWidth();
    }

    for (int i = 0; i < children.size(); i++) {
        if (children.at(i)->objectName() != "labeledLabel")
            continue;

        ((LabeledLabel*) children[i])->setAlignment(max);
    }
}

HardwareProviderPlugin::HardwareProviderPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::HardwareProviderPlugin)
{
    m_ui->setupUi(this);
    showFilter(false);
    setPluginEnabled(false);

    int cnt = sizeof(valueMap) / sizeof(valueMap[0]);
    for (int i = 0; i < cnt; i++)
        m_values[valueMap[i]] = values[i];   

    connect(
        m_ui->tree,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(showComponent()));
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
    // TODO LMI_DiskDrive*

    Pegasus::Array<Pegasus::CIMInstance> array;
    int size = 0;

    try {
        // processor
        array = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_Processor"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                    );

        unsigned int cnt = array.size();
        for (unsigned int i = 0; i < cnt; i++, size++) {
            data->push_back(new std::vector<Pegasus::CIMInstance>());
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[size]);
            vector->push_back(array[i]);

            Pegasus::Array<Pegasus::CIMObject> proc_capab =
                    m_client->associators(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        array[i].getPath(),
                        Pegasus::CIMName(),
                        Pegasus::CIMName("LMI_ProcessorCapabilities")
                        );
            vector->push_back(Pegasus::CIMInstance(proc_capab[0]));

            Pegasus::Array<Pegasus::CIMObject> cache =
                    m_client->associators(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        array[i].getPath(),
                        Pegasus::CIMName(),
                        Pegasus::CIMName("LMI_ProcessorCacheMemory")
                        );
            for (unsigned int j = 0; j < cache.size(); j++)
                vector->push_back(Pegasus::CIMInstance(cache[j]));
        }

        // memory
        array = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_PhysicalMemory"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                    );

        cnt = array.size();
        for (unsigned int i = 0; i < cnt; i++, size++) {
            data->push_back(new std::vector<Pegasus::CIMInstance>());
            std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[size]);
            vector->push_back(array[i]);

            Pegasus::Array<Pegasus::CIMObject> memory =
                    m_client->associators(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        array[i].getPath(),
                        Pegasus::CIMName(),
                        Pegasus::CIMName("LMI_Memory")
                        );
            vector->push_back(Pegasus::CIMInstance(memory[0]));
        }

        const char* devices[] = {
            "LMI_PCIDevice",
            "LMI_PCIBridge",
            "LMI_Chassis",
            "LMI_PortPhysicalConnector",
            "LMI_Battery"
        };
        int device_cnt = sizeof(devices) / sizeof(devices[0]);
        for (int j = 0; j < device_cnt; j++) {

            array = m_client->enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName(devices[j]),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                        );

            std::vector<Pegasus::CIMInstance> *vector = new std::vector<Pegasus::CIMInstance>();
            data->push_back(vector);

            cnt = array.size();
            for (unsigned int i = 0; i < cnt; i++, size++) {
                vector->push_back(array[i]);
            }
        }
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }

    emit doneFetchingData(data);
}

void HardwareProviderPlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    m_ui->tree->clear();

    try {
        std::vector<Pegasus::CIMInstance> *vector;
        unsigned int pos = 0;
        bool pci_device = true;
        while (pos < data->size()) {
            vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[pos++]);
            if (vector->empty())
                continue;
            if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Processor"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                QTreeWidgetItem *item = new QTreeWidgetItem();
                std::string name = getPropertyOfInstance(tmp[0], "ElementName");
                item->setText(0, name.c_str());
                item->setToolTip(0, name.c_str());
                topLevelNode("Processor")->addChild(item);

                m_processor.push_back(tmp);
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_PhysicalMemory"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                QTreeWidgetItem *item = new QTreeWidgetItem();
                std::string name = getPropertyOfInstance(tmp[0], "ElementName") + " " + getPropertyOfInstance(tmp[0], "PartNumber");
                item->setText(0, name.c_str());
                item->setToolTip(0, name.c_str());
                topLevelNode("Memory")->addChild(item);

                m_memory.push_back(tmp);
            } else if ((pci_device = (*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_PCIDevice")))
                       || (*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_PCIBridge"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = getPropertyOfInstance(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    Pegasus::Uint32 prop_ind = tmp[i].findProperty("ClassCode");
                    topLevelNode(decodeValues(tmp[i].getProperty(prop_ind)))->addChild(item);
                }
                if (pci_device)
                    m_pci_device = tmp;
                else
                    m_pci_bridge = tmp;
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Chassis"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = getPropertyOfInstance(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Chassis")->addChild(item);
                }
                m_chassis = tmp;
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_PortPhysicalConnector"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = getPropertyOfInstance(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Physical port")->addChild(item);
                }
                m_port = tmp;
            } else if ((*vector)[0].getClassName().equal(Pegasus::CIMName("LMI_Battery"))) {
                std::vector<Pegasus::CIMInstance> tmp = *((std::vector<Pegasus::CIMInstance> *) (*data)[pos - 1]);

                for (unsigned int i = 0; i < tmp.size(); i++) {
                    QTreeWidgetItem *item = new QTreeWidgetItem();
                    std::string name = getPropertyOfInstance(tmp[i], "Name");
                    item->setText(0, name.c_str());
                    item->setToolTip(0, name.c_str());

                    topLevelNode("Battery")->addChild(item);
                }
                m_battery = tmp;
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    m_changes_enabled = true;

    for (unsigned int i = 0; i < data->size(); i++)
        delete ((std::vector<Pegasus::CIMInstance> *) (*data)[i]);
}

void HardwareProviderPlugin::showComponent()
{
    if (m_ui->tree->selectedItems().empty())
        return;

    QTreeWidgetItem *item = m_ui->tree->selectedItems()[0];
    std::string parent = item->parent()->text(0).toStdString();
    std::string name = item->text(0).toStdString();

    QObjectList list = m_ui->device_box->children();

    // BUG remove all
    for (int i = list.size() - 1; i >= 0; i--) {
        m_ui->device_box->layout()->removeWidget(qobject_cast<QWidget*>(list[i]));
        delete list[i];
    }
    m_ui->device_box->setLayout(new QFormLayout());

    if (parent == "Processor") {
        for (unsigned int i = 0; i < m_processor.size(); i++) {
            std::vector<Pegasus::CIMInstance> proc = m_processor[i];
            if (name != getPropertyOfInstance(proc[0], "ElementName"))
                continue;

            fillProcessor(proc);
            break;
        }
    } else if (parent == "Memory") {
        name = name.substr(name.rfind(" ") + 1);
        for (unsigned int i = 0; i < m_memory.size(); i++) {
            std::vector<Pegasus::CIMInstance> mem = m_memory[i];

            if (name != getPropertyOfInstance(mem[0], "PartNumber"))
                continue;

            fillMemory(mem);
            break;
        }
    } else if (parent == "Battery") {
        for (unsigned int i = 0; i < m_battery.size(); i++) {
            Pegasus::CIMInstance batt = m_battery[i];

            if (name != getPropertyOfInstance(batt, "Name"))
                continue;

            fillBattery(batt);
            break;
        }
    } else if (arrayContains<const char*>(class_code_values,
                        sizeof(class_code_values) / sizeof(class_code_values[0]), parent.c_str())) {
        for (unsigned int i = 0; i < m_pci_device.size(); i++) {
            Pegasus::CIMInstance pci = m_pci_device[i];

            if (name != getPropertyOfInstance(pci, "Name"))
                continue;

            fillPCI(pci);
            return;
        }
        for (unsigned int i = 0; i < m_pci_bridge.size(); i++) {
            Pegasus::CIMInstance bridge = m_pci_bridge[i];

            if (name != getPropertyOfInstance(bridge, "Name"))
                continue;

            fillPCI(bridge);
            break;
        }
    } else if (parent == "Chassis") {
        for (unsigned int i = 0; i < m_chassis.size(); i++) {
            Pegasus::CIMInstance chassis = m_chassis[i];

            if (name != getPropertyOfInstance(chassis, "Name"))
                continue;

            fillChassis(chassis);
            break;
        }
    } else if (parent == "Physical port") {
        for (unsigned int i = 0; i < m_port.size(); i++) {
            Pegasus::CIMInstance port = m_port[i];

            if (name != getPropertyOfInstance(port, "Name"))
                continue;

            fillPort(port);
            break;
        }
    }
}

Q_EXPORT_PLUGIN2(hardwareProvider, HardwareProviderPlugin)
