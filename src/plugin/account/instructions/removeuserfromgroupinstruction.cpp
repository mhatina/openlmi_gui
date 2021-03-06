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

#include "removeuserfromgroupinstruction.h"
#include "logger.h"
#include "cimvalue.h"

#include <sstream>

RemoveUserFromGroupInstruction::RemoveUserFromGroupInstruction(
    CIMClient *client, String name, Pegasus::CIMValue user_id) :
    GroupInstruction(client, "remove_user_from_group", name, user_id)
{
}

IInstruction::Subject RemoveUserFromGroupInstruction::getSubject()
{
    return IInstruction::GROUP;
}

String RemoveUserFromGroupInstruction::toString()
{
    std::stringstream ss;
    ss << "identity = acc.associators(ResultClass=\"LMI_Identity\")[0]\n"
       << "for mog in identity.references(ResultClass=\"LMI_MemberOfGroup\"):\n"
       << "\tif mog.Collection == gr.path:\n"
       << "\t\tmog.delete()\n";
    return ss.str();
}

void RemoveUserFromGroupInstruction::run()
{
    try {
        Pegasus::Array<Pegasus::CIMInstance> members =
            m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_MemberOfGroup"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );

        Pegasus::CIMObjectPath member;
        int cnt = members.size();
        for (int i = 0; i < cnt; i++) {
            int ind = members[i].findProperty("Member");
            Pegasus::CIMProperty member_prop = members[i].getProperty(ind);
            ind  = members[i].findProperty("Collection");
            Pegasus::CIMProperty collection_prop = members[i].getProperty(ind);
            String member_property(CIMValue::to_string(member_prop.getValue()));
            String collection_property = CIMValue::to_string(collection_prop.getValue());
            if (member_property.find("LMI:UID:" + CIMValue::to_string(
                                         m_value)) != String::npos
                && collection_property.find(m_name) != String::npos) {
                member = members[i].getPath();
                break;
            }
        }

        m_client->deleteInstance(
            Pegasus::CIMNamespaceName("root/cimv2"),
            member
        );
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_string(ex.getMessage()));
    }
}
