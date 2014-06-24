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

#include "groupinstruction.h"

GroupInstruction::GroupInstruction(CIMClient *client, std::string instruction, std::string name, Pegasus::CIMValue value) :
    IInstruction(instruction, value),
    m_client(client),
    m_name(name)
{
}

GroupInstruction::GroupInstruction(CIMClient *client, std::string instruction, std::string name) :
    IInstruction(instruction),
    m_client(client),
    m_name(name)
{
}

GroupInstruction::~GroupInstruction()
{
}

Pegasus::CIMInstance GroupInstruction::getGroup()
{
    Pegasus::Array<Pegasus::CIMObject> groups;
    groups = m_client->execQuery(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::String("WQL"),
                Pegasus::String(std::string("SELECT * FROM LMI_Group WHERE Name = \"" + m_name + "\"").c_str())
                );

    if (groups.size() != 1) {
        throw Pegasus::Exception(std::string("No group with name: " + m_name + "\n").c_str());
    }

    return Pegasus::CIMInstance(groups[0]);
}
