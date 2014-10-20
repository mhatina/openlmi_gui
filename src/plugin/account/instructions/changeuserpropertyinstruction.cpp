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

#include "changeuserpropertyinstruction.h"
#include "account.h"
#include "logger.h"
#include "lmiwbem_value.h"

#include <sstream>

ChangeUserPropertyInstruction::ChangeUserPropertyInstruction(CIMClient *client,
        std::string property, std::string name, Pegasus::CIMValue value) :
    AccountInstruction(client, property, name, value)
{
}

bool ChangeUserPropertyInstruction::equals(ChangeUserPropertyInstruction
        *instruction)
{
    if (instruction->m_instruction == m_instruction
        && instruction->m_value.equal(m_value)
        && instruction->m_name == m_name) {
        return true;
    }

    return false;
}

IInstruction::Subject ChangeUserPropertyInstruction::getSubject()
{
    return IInstruction::ACCOUNT;
}

std::string ChangeUserPropertyInstruction::toString()
{
    std::stringstream ss;
    ss << "acc." << m_instruction << " = \"" << CIMValue::to_std_string(
           m_value) << "\"\n";
    return ss.str();
}

void ChangeUserPropertyInstruction::run()
{
    try {
        Pegasus::CIMInstance user(getUser());
        Pegasus::Uint32 propInd = user.findProperty(Pegasus::CIMName(
                                      m_instruction.c_str()));
        Pegasus::CIMProperty prop(user.getProperty(propInd));
        prop.setValue(m_value);
        m_client->modifyInstance(
            Pegasus::CIMNamespaceName("root/cimv2"),
            user,
            false
        );
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}
