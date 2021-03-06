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

#ifndef GETINSTRUCTION_H
#define GETINSTRUCTION_H

#include "instructions/instruction.h"
#include "lmi_string.h"

/**
 * @brief The GetInstruction class
 *
 * Get service.
 */
class GetInstruction : public IInstruction
{
private:
    String m_name;

public:
    /**
     * @brief Constructor
     * @param name -- name of service
     */
    GetInstruction(String name);
    IInstruction::Subject getSubject();
    String toString();
    void run();
};

#endif // GETINSTRUCTION_H
