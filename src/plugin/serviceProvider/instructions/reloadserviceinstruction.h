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

#ifndef RELOADSERVICEINSTRUCTION_H
#define RELOADSERVICEINSTRUCTION_H

#include "serviceinstruction.h"

/**
 * @brief The ReloadServiceInstruction class
 *
 * Reload service.
 */
class ReloadServiceInstruction : public ServiceInstruction
{
public:
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param name -- name of service
     */
    ReloadServiceInstruction(CIMClient *client, std::string name);
    IInstruction::Subject getSubject();
    std::string toString();
    void run();
};

#endif // RELOADSERVICEINSTRUCTION_H
