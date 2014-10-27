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

#include "cimvalue.h"
#include "logger.h"
#include "updatepackageinstruction.h"

#include <sstream>

UpdatePackageInstruction::UpdatePackageInstruction(CIMClient *client,
        Pegasus::CIMInstance package) :
    SoftwareInstruction(client, "update_package", package)
{
}

IInstruction::Subject UpdatePackageInstruction::getSubject()
{
    return IInstruction::SOFTWARE;
}

std::string UpdatePackageInstruction::toString()
{
    std::stringstream ss;
    ss << "service = c.root.cimv2.LMI_SoftwareInstallationService.first_instance()\n"
       << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
       << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
       << "cs = c.root.cimv2.PG_ComputerSystem.first_instance_name()\n"
       << "ret = service.SyncInstallFromSoftwareIdentity(\n"
       << "\tSource=identity,\n"
       << "\tTarget=cs,\n"
       << "\tInstallOptions=[5]       # [Update]\n"
       << "\t# to force update, when package is not installed\n"
       << "\t#InstallOptions=[4, 5]   # [Install, Update]\n"
       << ")\n"
       << "print \"installation \" + (\"successful\" if rval == 0 else \"failed\")\n";
    return ss.str();
}

void UpdatePackageInstruction::run()
{
    try {
        invokeInstallMethod(5); // Update
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}

