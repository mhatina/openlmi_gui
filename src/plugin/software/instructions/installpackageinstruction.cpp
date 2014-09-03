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

#include "installpackageinstruction.h"
#include "lmiwbem_value.h"
#include "logger.h"

#include <sstream>

InstallPackageInstruction::InstallPackageInstruction(CIMClient *client, Pegasus::CIMInstance package, bool synchronous) :
    SoftwareInstruction(client, "install_package", package),
    m_synchronous(synchronous)
{
}

IInstruction::Subject InstallPackageInstruction::getSubject()
{
    return IInstruction::SOFTWARE;
}

std::string InstallPackageInstruction::toString()
{
    std::stringstream ss;
    if (m_synchronous) {
        ss << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
           << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
           << "cs = c.root.cimv2.PG_ComputerSystem.first_instance_name()\n"
           << "installed_assoc = c.root.cimv2.LMI_InstalledSoftwareIdentity.create_instance(\n"
           << "\tproperties={\n"
           << "\t\t\"InstalledSoftware\" : identity,\n"
           << "\t\t\"System\"            : cs\n"
           << "\t})\n";
    } else {
        ss << "service = c.root.cimv2.LMI_SoftwareInstallationService.first_instance()\n"
           << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
           << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
           << "cs = c.root.cimv2.PG_ComputerSystem.first_instance_name()\n"
           << "ret = service.InstallFromSoftwareIdentity(\n"
           << "\tSource=identity,\n"
           << "\tTarget=cs,\n"
           << "\t# these options request to install available, not installed package\n"
           << "\tInstallOptions=[4]     # [Install]\n"
           << "\t# this will force installation if package is already installed\n"
           << "\t# (possibly in different version)\n"
           << "\t#InstallOptions=[4, 3] # [Install, Force installation]\n"
           << ")\n";
    }
    return ss.str();
}

void InstallPackageInstruction::run()
{
    try {
        if (m_synchronous) {
            Pegasus::CIMInstance package("LMI_InstalledSoftwareIdentity");
            package.addProperty(Pegasus::CIMProperty(
                                Pegasus::CIMName("InstalledSoftware"),
                                Pegasus::CIMValue(m_instance)
                                )
                       );

            Pegasus::Array<Pegasus::CIMInstance> system = m_client->enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("CIM_ComputerSystem"),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                        );
            package.addProperty(Pegasus::CIMProperty(
                                   Pegasus::CIMName("System"),
                                   Pegasus::CIMValue(system[0])
                                   )
                               );

            m_client->createInstance(
                       Pegasus::CIMNamespaceName("root/cimv2"),
                       package
                       );
        } else {
            invokeInstallMethod(4); // Install
        }
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
    }
}

