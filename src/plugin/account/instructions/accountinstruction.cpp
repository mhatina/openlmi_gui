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

#include "accountinstruction.h"

AccountInstruction::AccountInstruction(CIMClient *client,
                                       String instruction, String name, Pegasus::CIMValue value):
    IInstruction(instruction, value),
    m_client(client),
    m_name(name)
{
}

AccountInstruction::AccountInstruction(CIMClient *client,
                                       String instruction, String name):
    IInstruction(instruction),
    m_client(client),
    m_name(name)
{
}

AccountInstruction::~AccountInstruction()
{
}

Pegasus::CIMInstance AccountInstruction::getUser()
{
    Pegasus::Array<Pegasus::CIMObject> users;
    users = m_client->execQuery(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::String("WQL"),
                String("SELECT * FROM LMI_Account WHERE Name = \"" + m_name
                                            + "\"")
            );

    if (users.size() != 1) {
        throw Pegasus::Exception(String("No user with name: " + m_name +
                                             "\n").c_str());
    }

    return Pegasus::CIMInstance(users[0]);
}

String AccountInstruction::getUserName()
{
    return m_name;
}
