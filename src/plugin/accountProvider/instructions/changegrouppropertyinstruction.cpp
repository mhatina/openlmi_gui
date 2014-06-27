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

#include "changegrouppropertyinstruction.h"
#include "accountprovider.h"
#include "lmiwbem_value.h"

#include <sstream>

ChangeGroupPropertyInstruction::ChangeGroupPropertyInstruction(CIMClient *client, std::string property, std::string name, Pegasus::CIMValue value) :
    GroupInstruction(client, property, name, value)
{
}

bool ChangeGroupPropertyInstruction::equals(ChangeGroupPropertyInstruction *instruction)
{
    if (instruction->m_instruction == m_instruction
            && instruction->m_value.equal(m_value)
            && instruction->m_name == m_name)
        return true;

    return false;
}

IInstruction::Subject ChangeGroupPropertyInstruction::getSubject()
{
    return IInstruction::GROUP;
}

std::string ChangeGroupPropertyInstruction::toString()
{
    std::stringstream ss;
    ss << "gr." << m_instruction << " = \"" << CIMValue::to_std_string(m_value) << "\"\n";
    return ss.str();
}

void ChangeGroupPropertyInstruction::run()
{
    try {
        Pegasus::CIMInstance group(getGroup());
        Pegasus::Uint32 propInd = group.findProperty(Pegasus::CIMName(m_instruction.c_str()));
        Pegasus::CIMProperty prop = group.getProperty(propInd);
        prop.setValue(m_value);
        m_client->modifyInstance(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    group,
                    false
                    );
    } catch (Pegasus::Exception &ex)
    {
        emit error(CIMValue::to_std_string(ex.getMessage()));
    }
}
