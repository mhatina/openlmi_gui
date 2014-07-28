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

#ifndef INSTALLPACKAGEINSTRUCTION_H
#define INSTALLPACKAGEINSTRUCTION_H

#include "instructions/softwareinstruction.h"
#include "lmiwbem_client.h"

class InstallPackageInstruction : public SoftwareInstruction
{
private:
    bool m_synchronous;

public:
    InstallPackageInstruction(CIMClient *client, Pegasus::CIMInstance package, bool synchronous);
    IInstruction::Subject getSubject();
    std::string toString();
    void run();
};

#endif // INSTALLPACKAGEINSTRUCTION_H
