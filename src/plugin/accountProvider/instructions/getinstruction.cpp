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

#include "getinstruction.h"
#include "instructions/instruction.h"

#include <sstream>

GetInstruction::GetInstruction(Subject s, std::string name, Subject save_to) :
    IInstruction("get"),
    m_subject(s),
    m_name(name),
    m_save_to(save_to)
{
}

GetInstruction::GetInstruction(IInstruction::Subject s, std::string name) :
    IInstruction("get"),
    m_subject(s),
    m_name(name),
    m_save_to(s)
{
}

IInstruction::Subject GetInstruction::getSubject()
{
    return m_save_to;
}

std::string GetInstruction::toString()
{
    std::stringstream ss;
    switch (m_subject) {
    case ACCOUNT:
        ss << "acc = c.root.cimv2.LMI_Account.first_instance({\"Name\": \"" << m_name << "\"})\n";
        return ss.str();
    case GROUP:
        ss << "gr = c.root.cimv2.LMI_Group.first_instance({\"Name\": \"" << m_name << "\"})\n";
        return ss.str();
    default:
        return "";
    }
}

void GetInstruction::run()
{
}
