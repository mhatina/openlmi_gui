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

#include "addusertogroupinstruction.h"
#include "logger.h"
#include "lmiwbem_value.h"

#include <sstream>

AddUserToGroupInstruction::AddUserToGroupInstruction(CIMClient *client, std::string name, Pegasus::CIMValue user_id) :
    GroupInstruction(client, "add_user_to_group", name, user_id)
{
}

IInstruction::Subject AddUserToGroupInstruction::getSubject()
{
    return IInstruction::GROUP;
}

std::string AddUserToGroupInstruction::toString()
{
    std::stringstream ss;
    ss << "identity = acc.first_associator(ResultClass=\"LMI_Identity\")\n"
       << "c.root.cimv2.LMI_MemberOfGroup.create_instance({\"Member\":identity.path, \"Collection\":gr.path})\n";
    return ss.str();
}

void AddUserToGroupInstruction::run()
{
    try {
        Pegasus::CIMInstance group(getGroup());
        Pegasus::CIMInstance member("LMI_MemberOfGroup");
        member.addProperty(Pegasus::CIMProperty(
                            Pegasus::CIMName("Collection"),
                            Pegasus::CIMValue(group)
                            )
                   );

        Pegasus::Array<Pegasus::CIMObject> identity = m_client->execQuery(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::String("WQL"),
                    Pegasus::String(std::string("SELECT * FROM LMI_Identity WHERE InstanceID = \"LMI:UID:" +
                                                CIMValue::to_std_string(m_value) + "\"").c_str())
                    );
        member.addProperty(Pegasus::CIMProperty(
                               Pegasus::CIMName("Member"),
                               Pegasus::CIMValue(identity[0].getPath())
                               )
                           );

        m_client->createInstance(
                   Pegasus::CIMNamespaceName("root/cimv2"),
                   member
                   );
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
    }
}
