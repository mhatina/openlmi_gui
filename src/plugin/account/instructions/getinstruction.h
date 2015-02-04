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

/**
 * @brief The GetInstruction class
 *
 * Get account/group. Depends on subject
 */
class GetInstruction : public IInstruction
{
private:
    IInstruction::Subject m_subject;
    String m_name;
    IInstruction::Subject m_save_to;

public:
    /**
     * @brief Constructor
     * @param s -- subject
     * @param name -- name of account/group
     * @param save_to -- save to account/group file
     */
    GetInstruction(IInstruction::Subject s, String name,
                   IInstruction::Subject save_to);
    /**
     * @brief Constructor
     * @param s -- subject
     * @param name -- name of account/group
     */
    GetInstruction(IInstruction::Subject s, String name);
    IInstruction::Subject getSubject();
    String toString();
    void run();
};

#endif // GETINSTRUCTION_H
