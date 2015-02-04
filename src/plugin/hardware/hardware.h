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

#ifndef HARDWARE_H
#define HARDWARE_H

#include "lmi_string.h"
#include "plugin.h"

#include <QtPlugin>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

const char *const valueMap[] = {
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "11", "12", "13", "14", "15",
    "16", "17", "18", "19", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30",
    "31", "43", "51", "52", "54", "57", "58",
    "59", "61", "62", "63", "64", "65", "67",
    "96", "97", "98", "99", "100", "101",
    "102", "103", "104", "105", "106", "107",
    "108", "109", "110", "111", "112", "113",
    "114", "115", "116", "118", "119", "120",
    "121", "122", "123", "124", "125", "128",
    "129", "130", "131", "132", "133", "134",
    "135", "136", "137", "138", "140", "141",
    "142", "143", "145", "146", "147", "148",
    "149", "150", "151", "152", "153", "154",
    "155", "156", "157", "158", "159", "162",
    "163", "166", "167", "168", "169", "170",
    "171", "172", "173", "192", "193", "194",
    "195", "196", "197", "198", "199", "200",
    "201", "202", "203", "204", "205", "207",
    "208", "209", "211", "213", "214", "215",
    "224", "225", "226", "227", "228", "229",
    "230", "231", "232", "256", "257", "258",
    "259", "260", "261", "262", "263", "264",
    "265", "266", "267", "268", "269", "270",
    "288", "289", "291", "292", "293", "295",
    "296", "297", "298", "299", "306", "307",
    "308"
};

const char *const values[] = {
    "fpu", "vme", "de", "pse", "tsc", "msr",
    "pae", "mce", "cx8", "apic", "sep",
    "mtrr", "pge", "mca", "cmov", "pat",
    "pse36", "pn", "clflush", "dts", "acpi",
    "mmx", "fxsr", "sse", "sse2", "ss", "ht",
    "tm", "ia64", "pbe", "syscall", "mp",
    "nx", "mmxext", "fxsr_opt", "pdpe1gb",
    "rdtscp", "lm", "3dnowext", "3dnow",
    "recovery", "longrun", "lrti", "cxmmx",
    "k6_mtrr", "cyrix_arr", "centaur_mcr",
    "k8", "k7", "p3", "p4", "constant_tsc",
    "up", "fxsave_leak", "arch_perfmon",
    "pebs", "bts", "syscall32", "sysenter32",
    "rep_good", "mfence_rdtsc",
    "lfence_rdtsc", "11ap", "nopl",
    "xtopology", "tsc_reliable",
    "nonstop_tsc", "clflush_monitor",
    "extd_apicid", "amd_dcm", "aperfmperf",
    "eagerfpu", "pni", "pclmulqdq", "dtes64",
    "monitor", "ds_cpl", "vmx", "smx", "est",
    "tm2", "ssse3", "cid", "fma", "cx16",
    "xtpr", "pdcm", "pcid", "dca", "sse4_1",
    "sse4_2", "x2apic", "movbe", "popcnt",
    "tsc_deadline_timer", "aes", "xsave",
    "osxsave", "avx", "f16c", "rdrand",
    "hypervisor", "rng", "rng_en", "ace",
    "ace_en", "ace2", "ace2_en", "phe",
    "phe_en", "pmm", "pmm_en", "lahf_lm",
    "cmp_legacy", "svm", "extapic",
    "cr8_legacy", "abm", "sse4a",
    "misalignsse", "3dnowprefetch", "osvw",
    "ibs", "xop", "skinit", "wdt", "lwp",
    "fma4", "tce", "nodeid_msr", "tbm",
    "topoext", "perfctr_core", "ida", "arat",
    "cpb", "epb", "xsaveopt", "pln", "pts",
    "dtherm", "hw_pstate", "tpr_shadow",
    "vnmi", "flexpriority", "ept", "vpid",
    "npt", "lbrv", "svm_lock", "nrip_save",
    "tsc_scale", "vmcb_clean", "flushbyasid",
    "decodeassists", "pausefilter",
    "pfthreshold", "fsgsbase", "tsc_adjust",
    "bmi1", "hle", "avx2", "smep", "bmi2",
    "erms", "invpcid", "rtm", "rdseed",
    "adx", "smap"
};

const char *const class_code_values[] = {
    "Pre 2.0", "Mass Storage", "Network",
    "Display", "Multimedia", "Memory",
    "Bridge", "Simple Communications",
    "Base Peripheral", "Input", "Docking Station",
    "Processor", "Serial Bus", "Wireless",
    "Intelligent I/O", "Satellite Communication",
    "Encryption/Decryption",
    "Data Acquisition and Signal Processing"
};

namespace Ui
{
class HardwarePlugin;
}

class HardwarePlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    bool m_changes_enabled;
    std::map<String, String> m_values;
    std::vector<Pegasus::CIMInstance> m_battery;
    std::vector<Pegasus::CIMInstance> m_chassis;    
    std::vector<Pegasus::CIMInstance> m_pci_bridge;
    std::vector<Pegasus::CIMInstance> m_pci_device;
    std::vector<Pegasus::CIMInstance> m_port;    
    std::vector<std::vector<Pegasus::CIMInstance> > m_disk_drive;
    std::vector<std::vector<Pegasus::CIMInstance> > m_memory;
    std::vector<std::vector<Pegasus::CIMInstance> > m_processor;
    Ui::HardwarePlugin *m_ui;

    QTreeWidgetItem *findTopLevelNode(String item_name);
    QTreeWidgetItem *topLevelNode(String item_name);
    String decodeValues(Pegasus::CIMProperty property);
    void clearComponentInfo();
    void fillBattery(Pegasus::CIMInstance battery);
    void fillChassis(Pegasus::CIMInstance chassis);
    void fillDisk(std::vector<Pegasus::CIMInstance> disk);
    void fillMemory(std::vector<Pegasus::CIMInstance> memory);
    void fillPCI(Pegasus::CIMInstance pci);
    void fillPort(Pegasus::CIMInstance port);
    void fillProcessor(std::vector<Pegasus::CIMInstance> processor);
    void setAlignment();

public:
    explicit HardwarePlugin();
    ~HardwarePlugin();
    virtual String getInstructionText();
    virtual String getLabel();
    virtual String getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

private slots:
    void showComponent();
};

#endif // HARDWARE_H
