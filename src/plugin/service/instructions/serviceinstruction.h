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

#ifndef SERVICEINSTRUCTION_H
#define SERVICEINSTRUCTION_H

#include "instructions/instruction.h"
#include "cimclient.h"

/**
 * @brief The ServiceInstruction class
 */
class ServiceInstruction : public IInstruction
{
protected:
    CIMClient *m_client;
    std::string m_name;

    Pegasus::CIMInstance getService();
    Pegasus::CIMValue invokeMethod(std::string method);

public:
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param instruction -- name of instruction
     * @param name -- name of service
     */
    ServiceInstruction(CIMClient *client, std::string instruction,
                       std::string name);
    /**
     * @brief Destructor
     */
    ~ServiceInstruction();
};

#endif // SERVICEINSTRUCTION_H
