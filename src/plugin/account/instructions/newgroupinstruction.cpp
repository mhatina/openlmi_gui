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
#include "newgroupinstruction.h"

#include <sstream>

NewGroupInstruction::NewGroupInstruction(CIMClient *client,
        NewGroupDialog *dialog) :
    GroupInstruction(client, "add_new_group", dialog->getName()),
    m_dialog(dialog)
{
}

IInstruction::Subject NewGroupInstruction::getSubject()
{
    return IInstruction::GROUP;
}

String NewGroupInstruction::toString()
{
    std::stringstream ss;
    ss << "cs = c.root.cimv2.PG_ComputerSystem.first_instance()\n"
       << "lams = c.root.cimv2.LMI_AccountManagementService.first_instance()\n"
       << "lams.CreateGroup(Name=\"" << m_dialog->getName() << "\", System=cs)\n";
    return ss.str();
}

void NewGroupInstruction::run()
{
    try {
        Pegasus::CIMObjectPath account_inst_name = m_client->enumerateInstanceNames(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_AccountManagementService")
                )[0];

        Pegasus::Array<Pegasus::CIMInstance> computer_systems =
            m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("PG_ComputerSystem"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );

        if (computer_systems.size() != 1) {
            Logger::getInstance()->critical("Invalid number of PG_ComputerSystem");
            return;
        }

        Pegasus::Array<Pegasus::CIMParamValue> in_param;
        Pegasus::Array<Pegasus::CIMParamValue> out_param;

        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("System"),
                            Pegasus::CIMValue(computer_systems[0].getPath())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Name"),
                            CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING, m_dialog->getName())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("SystemAccount"),
                            Pegasus::CIMValue(m_dialog->isSystemGroup())
                        ));

        m_client->invokeMethod(
            Pegasus::CIMNamespaceName("root/cimv2"),
            account_inst_name,
            Pegasus::CIMName("CreateGroup"),
            in_param,
            out_param
        );
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_string(ex.getMessage()));
    }
}
