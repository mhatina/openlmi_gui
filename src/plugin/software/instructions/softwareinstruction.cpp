#include "softwareinstruction.h"

SoftwareInstruction::SoftwareInstruction(CIMClient *client, std::string instruction, Pegasus::CIMInstance instance) :
    IInstruction(instruction),
    m_client(client),
    m_instance(instance)
{
    Pegasus::Uint32 prop_ind = instance.findProperty("InstalledSoftware");
    if (prop_ind != Pegasus::PEG_NOT_FOUND) {
        Pegasus::CIMProperty property = instance.getProperty(prop_ind);

        std::string name =  (std::string) property.getValue().toString().getCString();

        int ch = name.rfind(":", name.rfind(":") - 1);
        name = name.substr(ch + 1, name.length() - ch - 2);
        m_name = name;
    } else {
        prop_ind = instance.findProperty("Caption");
        Pegasus::CIMProperty property = instance.getProperty(prop_ind);

        m_name = (std::string) property.getValue().toString().getCString();
    }
}

Pegasus::CIMValue SoftwareInstruction::invokeInstallMethod(Pegasus::Uint16 install_option)
{
    Pegasus::CIMInstance install_service =
            m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_SoftwareInstallationService"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
                )[0];
    Pegasus::Array<Pegasus::CIMParamValue> in_param;
    Pegasus::Array<Pegasus::CIMParamValue> out_param;


    Pegasus::Array<Pegasus::Uint16> install_options;
    install_options.append(Pegasus::Uint16(install_option));
    in_param.append(Pegasus::CIMParamValue(
                        Pegasus::String("InstallOptions"),
                        Pegasus::CIMValue(install_options)
                        ));

    Pegasus::Uint32 prop_ind = m_instance.findProperty("InstalledSoftware");
    Pegasus::CIMValue value_source = m_instance.getProperty(prop_ind).getValue();
    Pegasus::CIMObjectPath source;
    value_source.get(source);

//    Pegasus::CIMInstance instance_source = m_client->getInstance(
//                Pegasus::CIMNamespaceName("root/cimv2"),
//                source
//                );

    in_param.append(Pegasus::CIMParamValue(
                        Pegasus::String("Source"),
                        Pegasus::CIMValue(source)));

    prop_ind = m_instance.findProperty("System");
    Pegasus::CIMValue value_target = m_instance.getProperty(prop_ind).getValue();
    Pegasus::CIMObjectPath target;
    value_target.get(target);

//    Pegasus::CIMInstance instance_target = m_client->getInstance(
//                Pegasus::CIMNamespaceName("root/cimv2"),
//                target
//                );

    in_param.append(Pegasus::CIMParamValue(
                        Pegasus::String("Target"),
                        Pegasus::CIMValue(target)));

    return m_client->invokeMethod(
                Pegasus::CIMNamespaceName("root/cimv2"),
                install_service.getPath(),
                Pegasus::CIMName("InstallFromSoftwareIdentity"),
                in_param,
                out_param);
}

std::string SoftwareInstruction::getName()
{
    return m_name;
}
