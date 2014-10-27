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
#include "newuserinstruction.h"

#include <sstream>

NewUserInstruction::NewUserInstruction(CIMClient *client,
                                       NewUserDialog *dialog) :
    AccountInstruction(client, "add_new_user", dialog->getName()),
    m_dialog(dialog)
{
}

IInstruction::Subject NewUserInstruction::getSubject()
{
    return IInstruction::ACCOUNT;
}

std::string NewUserInstruction::toString()
{
    std::stringstream ss;
    ss << "cs = c.root.cimv2.PG_ComputerSystem.first_instance()\n"
       << "lams = c.root.cimv2.LMI_AccountManagementService.first_instance()\n"
       << "lams.CreateAccount(Name=\"" << m_dialog->getName() << "\", System=cs)\n";
    return ss.str();
}

void NewUserInstruction::run()
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
                            Pegasus::String("GECOS"),
                            CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING, m_dialog->getGecos())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("HomeDirectory"),
                            CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING, m_dialog->getHomeDir())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("DontCreateHome"),
                            Pegasus::CIMValue(!m_dialog->createHome())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Shell"),
                            CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING, m_dialog->getShell())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("SystemAccount"),
                            Pegasus::CIMValue(m_dialog->isSystemAccount())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Password"),
                            CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING, m_dialog->getPasswd())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("DontCreateGroup"),
                            Pegasus::CIMValue(!m_dialog->createGroup())
                        ));
        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("PasswordIsPlain"),
                            Pegasus::CIMValue(false)
                        ));

        m_client->invokeMethod(
            Pegasus::CIMNamespaceName("root/cimv2"),
            account_inst_name,
            Pegasus::CIMName("CreateAccount"),
            in_param,
            out_param
        );
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}
