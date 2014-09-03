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

#include "deleteuserinstruction.h"
#include "lmiwbem_value.h"
#include "logger.h"

DeleteUserInstruction::DeleteUserInstruction(CIMClient *client, std::string name) :
    AccountInstruction(client, "delete_user", name)
{
}

IInstruction::Subject DeleteUserInstruction::getSubject()
{
    return IInstruction::ACCOUNT;
}

std::string DeleteUserInstruction::toString()
{
    return "acc.DeleteUser()\n";
}

void DeleteUserInstruction::run()
{
    try {
        Pegasus::CIMInstance user(getUser());
        Pegasus::Array<Pegasus::CIMParamValue> in_param;
        Pegasus::Array<Pegasus::CIMParamValue> out_param;

        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("DontDeleteHomeDirectory"),
                            Pegasus::CIMValue(false)
                            ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("DontDeleteGroup"),
                            Pegasus::CIMValue(true)
                            ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Force"),
                            Pegasus::CIMValue(true)
                            ));

        Pegasus::CIMValue ret = m_client->invokeMethod(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    user.getPath(),
                    Pegasus::CIMName("DeleteUser"),
                    in_param,
                    out_param
                    );

        // home dir probably just dont exist
        //  4097 == "Unable to delete Home Directory"
        if (ret.equal(CIMValue::to_cim_value(Pegasus::CIMTYPE_UINT32, "4097"))) {
            Pegasus::CIMValue dontDeleteHome(true);
            in_param[0].setValue(dontDeleteHome);
            ret = m_client->invokeMethod(
                                Pegasus::CIMNamespaceName("root/cimv2"),
                                user.getPath(),
                                Pegasus::CIMName("DeleteUser"),
                                in_param,
                                out_param
                                );
        }
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(CIMValue::to_std_string(ex.getMessage()));
    }
}
