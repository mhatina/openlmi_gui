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

#include "serviceinstruction.h"

ServiceInstruction::ServiceInstruction(CIMClient *client,
                                       String instruction, String name) :
    IInstruction(instruction),
    m_client(client),
    m_name(name)
{
}

ServiceInstruction::~ServiceInstruction()
{

}

Pegasus::CIMInstance ServiceInstruction::getService()
{
    Pegasus::Array<Pegasus::CIMObject> services;
    services = m_client->execQuery(
                   Pegasus::CIMNamespaceName("root/cimv2"),
                   Pegasus::String("WQL"),
                   String("SELECT * FROM LMI_Service WHERE Name = \"" + m_name + "\"")
               );

    if (services.size() != 1) {
        throw Pegasus::Exception(String("No service with name: " + m_name + "\n"));
    }

    return Pegasus::CIMInstance(services[0]);
}

Pegasus::CIMValue ServiceInstruction::invokeMethod(String method)
{
    Pegasus::CIMInstance service(getService());
    Pegasus::Array<Pegasus::CIMParamValue> in_param;
    Pegasus::Array<Pegasus::CIMParamValue> out_param;

    return m_client->invokeMethod(
               Pegasus::CIMNamespaceName("root/cimv2"),
               service.getPath(),
               Pegasus::CIMName(method.asPegasusString()),
               in_param,
               out_param
           );
}
