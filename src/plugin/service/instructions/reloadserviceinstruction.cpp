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

#include "cimvalue.h"
#include "logger.h"
#include "reloadserviceinstruction.h"

ReloadServiceInstruction::ReloadServiceInstruction(CIMClient *client,
        std::string name) :
    ServiceInstruction(client, "reload_service", name)
{
}

IInstruction::Subject ReloadServiceInstruction::getSubject()
{
    return IInstruction::SERVICE;
}

std::string ReloadServiceInstruction::toString()
{
    return "srv.ReloadService()\n";
}

void ReloadServiceInstruction::run()
{
    try {
        Pegasus::CIMValue ret = invokeMethod("ReloadService");

        if (!ret.equal(Pegasus::CIMValue(Pegasus::Uint32(0)))) {
            Logger::getInstance()->info("Unable to reload service.");
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}
