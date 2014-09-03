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

#include "disablerepoinstruction.h"
#include "lmiwbem_value.h"
#include "logger.h"

#include <sstream>

DisableRepoInstruction::DisableRepoInstruction(CIMClient *client, Pegasus::CIMInstance repo) :
    SoftwareInstruction(client, "disable_repo", repo)
{
}

IInstruction::Subject DisableRepoInstruction::getSubject()
{
    return IInstruction::SOFTWARE;
}

std::string DisableRepoInstruction::toString()
{
    std::stringstream ss;
    ss << "repo = c.root.cimv2.LMI_SoftwareIdentityResource.first_instance_name(\n"
       << "\tkey=\"Name\",\n"
       << "\tvalue=\"" + m_name + "\")\n"
       << "# disable repository\n"
       << "repo.to_instance().RequestStateChange(\n"
       << "\tRequestedState=c.root.cimv2.LMI_SoftwareIdentityResource. \\\n"
       << "\t\tRequestedStateValues.Disabled)\n";
    return ss.str();
}

void DisableRepoInstruction::run()
{
    try {
        Pegasus::Array<Pegasus::CIMParamValue> in_param;
        Pegasus::Array<Pegasus::CIMParamValue> out_param;

        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("RequestedState"),
                            Pegasus::CIMValue(Pegasus::Uint16(3)) // Disable
                            ));

        m_client->invokeMethod(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    m_instance.getPath(),
                    Pegasus::CIMName("RequestStateChange"),
                    in_param,
                    out_param);
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
    }
}
