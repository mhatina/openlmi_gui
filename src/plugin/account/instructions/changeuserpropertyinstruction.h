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

#ifndef CHANGEUSERPROPERTYINSTRUCTION_H
#define CHANGEUSERPROPERTYINSTRUCTION_H

#include "accountinstruction.h"

/**
 * @brief The ChangeUserPropertyInstruction class
 *
 * Change user's property.
 */
class ChangeUserPropertyInstruction : public AccountInstruction
{
public:
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param property -- property name
     * @param name -- name of user
     * @param value -- value of change
     */
    ChangeUserPropertyInstruction(CIMClient *client, String property,
                                  String name, Pegasus::CIMValue value);
    /**
     * @brief Compare two instructions
     * @param instruction -- another instruction
     * @return true if instructions equal; else false
     */
    bool equals(ChangeUserPropertyInstruction *instruction);
    IInstruction::Subject getSubject();
    String toString();
    void run();
};

#endif // CHANGEUSERPROPERTYINSTRUCTION_H
