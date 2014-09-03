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

#include "deletegroupinstruction.h"
#include "lmiwbem_value.h"
#include "logger.h"

DeleteGroupInstruction::DeleteGroupInstruction(CIMClient *client, std::string name) :
    GroupInstruction(client, "delete_group", name)
{
}

IInstruction::Subject DeleteGroupInstruction::getSubject()
{
    return IInstruction::GROUP;
}

std::string DeleteGroupInstruction::toString()
{
    return "gr.DeleteGroup()\n";
}

void DeleteGroupInstruction::run()
{
    try {
        Pegasus::CIMInstance group(getGroup());
        Pegasus::Array<Pegasus::CIMParamValue> in_param;
        Pegasus::Array<Pegasus::CIMParamValue> out_param;

        Pegasus::CIMValue ret = m_client->invokeMethod(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    group.getPath(),
                    Pegasus::CIMName("DeleteGroup"),
                    in_param,
                    out_param
                    );        
        if (ret.equal(CIMValue::to_cim_value(Pegasus::CIMTYPE_UINT32, "4097")))
            Logger::getInstance()->info("Unable to delete group: " + m_name + ". Group is primary group of a user.");
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
    }
}
