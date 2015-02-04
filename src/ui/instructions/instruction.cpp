/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "instruction.h"
#include "logger.h"

IInstruction::IInstruction(String instruction, Pegasus::CIMValue value) :
    m_value(value),
    m_instruction(instruction)
{
    Logger::getInstance()->debug("IInstruction::IInstruction(String instruction, Pegasus::CIMValue value)");
}

IInstruction::IInstruction(String instruction) :
    m_value(),
    m_instruction(instruction)
{
    Logger::getInstance()->debug("IInstruction::IInstruction(String instruction)");
}

IInstruction::~IInstruction()
{
    Logger::getInstance()->debug("IInstruction::~IInstruction()");
}

Pegasus::CIMValue IInstruction::getValue()
{
    Logger::getInstance()->debug("IInstruction::getValue()");
    return m_value;
}

String IInstruction::getInstructionName()
{
    Logger::getInstance()->debug("IInstruction::getInstructionName()");
    return m_instruction;
}
