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

#ifndef GROUPINSTRUCTION_H
#define GROUPINSTRUCTION_H

#include "instructions/instruction.h"
#include "cimclient.h"

/**
 * @brief The GroupInstruction class
 */
class GroupInstruction : public IInstruction
{
protected:
    CIMClient *m_client; /**< @brief see CIMClient */
    std::string m_name; /**< @brief Name of group */

    /**
     * @brief Getter
     * @return Pegasus::CIMInstance representation of group
     */
    Pegasus::CIMInstance getGroup();

public:
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param instruction -- name of instruction
     * @param name -- name of group
     * @param value -- new property value
     */
    GroupInstruction(CIMClient *client, std::string instruction, std::string name,
                     Pegasus::CIMValue value);
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param instruction -- name of instruction
     * @param name -- name of group
     */
    GroupInstruction(CIMClient *client, std::string instruction, std::string name);
    /**
     * @brief Destructor
     */
    ~GroupInstruction();
    std::string getGroupName();
};

#endif // GROUPINSTRUCTION_H
