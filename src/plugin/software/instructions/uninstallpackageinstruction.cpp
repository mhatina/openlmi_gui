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

#include "lmiwbem_value.h"
#include "logger.h"
#include "uninstallpackageinstruction.h"

#include <sstream>

UninstallPackageInstruction::UninstallPackageInstruction(CIMClient *client,
        Pegasus::CIMInstance package, bool synchronous) :
    SoftwareInstruction(client, "uninstall_package", package),
    m_synchronous(synchronous)
{
}

IInstruction::Subject UninstallPackageInstruction::getSubject()
{
    return IInstruction::SOFTWARE;
}

std::string UninstallPackageInstruction::toString()
{
    std::stringstream ss;

    if (m_synchronous)     {
        ss << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
           << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
           << "installed_assocs = identity.to_instance().reference_names(\n"
           << "\tRole=\"InstalledSoftware\",\n"
           << "\tResultClass=\"LMI_InstalledSoftwareIdentity\")\n"
           << "if len(installed_assocs) > 0:\n"
           << "\tfor assoc in installed_assocs:\n"
           << "\t\tassoc.to_instance().delete()\n"
           << "\t\tprint(\"deleted %s\" % assoc.InstalledSoftware.InstanceID)\n"
           << "else:\n"
           << "\tprint(\"no package removed\")\n";
    } else {
        ss << "service = c.root.cimv2.LMI_SoftwareInstallationService.first_instance()\n"
           << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
           << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
           << "cs = c.root.cimv2.PG_ComputerSystem.first_instance_name()\n"
           << "ret = service.InstallFromSoftwareIdentity(\n"
           << "\tSource=identity,\n"
           << "\tTarget=cs,\n"
           << "\tInstallOptions=[9])  # [Uninstall]\n";
    }

    return ss.str();
}

void UninstallPackageInstruction::run()
{
    try {
        if (m_synchronous) {
            m_client->deleteInstance(
                Pegasus::CIMNamespaceName("root/cimv2"),
                m_instance.getPath());
        } else {
            invokeInstallMethod(9); // Uninstall
        }

    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}

