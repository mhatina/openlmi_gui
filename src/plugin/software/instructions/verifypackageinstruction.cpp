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
#include "verifypackageinstruction.h"

#include <sstream>

VerifyPackageInstruction::VerifyPackageInstruction(CIMClient *client,
        Pegasus::CIMInstance package) :
    SoftwareInstruction(client, "verify_package", package)
{
}

IInstruction::Subject VerifyPackageInstruction::getSubject()
{
    return IInstruction::SOFTWARE;
}

std::string VerifyPackageInstruction::toString()
{
    std::stringstream ss;
    ss << "service = c.root.cimv2.LMI_SoftwareInstallationService.first_instance()\n"
       << "identity = c.root.cimv2.LMI_SoftwareIdentity.new_instance_name(\n"
       << "\t{\"InstanceID\" : \"LMI:LMI_SoftwareIdentity:" + m_name + "\"})\n"
       << "results = service.VerifyInstalledIdentity(\n"
       << "\tSource=identity,\n"
       << "\tTarget=ns.PG_ComputerSystem.first_instance_name())\n"
       << "nevra = (    identity.ElementName if isinstance(identity, LMIInstance)\n"
       << "\telse identity.InstanceID[len('LMI:LMI_SoftwareIdentity:'):])\n"
       << "if results.rval != 4096:\n"
       << "\tmsg = 'failed to verify identity \"%s (rval=%d)\"' % (nevra, results.rval)\n"
       << "\tif results.errorstr:\n"
       << "\t\tmsg += ': ' + results.errorstr\n"
       << "\traise Exception(msg)\n\n"
       << "job = results.rparams['Job'].to_instance()\n\n"
       << "# wait by polling or listening for indication\n"
       << "wait_for_job_finished(job)\n\n"
       << "if not LMIJob.lmi_is_job_completed(job):\n"
       << "\tmsg = 'failed to verify package \"%s\"' % nevra\n"
       << "\tif job.ErrorDescription:\n"
       << "\t\tmsg += ': ' + job.ErrorDescription\n"
       << "\traise Exception(msg)\n\n"
       << "# get the failed files\n"
       << "failed = job.associators(\n"
       << "\tAssocClass=\"LMI_AffectedSoftwareJobElement\",\n"
       << "\tRole='AffectingElement',\n"
       << "\tResultRole='AffectedElement',\n"
       << "\tResultClass='LMI_SoftwareIdentityFileCheck')\n"
       << "for iname in failed:\n"
       << "\tprint iname.Name    # print their paths\n";
    return ss.str();
}

void VerifyPackageInstruction::run()
{
    try {
        Pegasus::CIMInstance install_service =
            m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_SoftwareInstallationService"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            )[0];
        Pegasus::Array<Pegasus::CIMParamValue> in_param;
        Pegasus::Array<Pegasus::CIMParamValue> out_param;


        Pegasus::Uint32 prop_ind = m_instance.findProperty("InstalledSoftware");
        Pegasus::CIMValue value_source = m_instance.getProperty(prop_ind).getValue();
        Pegasus::CIMObjectPath source;
        value_source.get(source);

        Pegasus::CIMInstance instance_source = m_client->getInstance(
                Pegasus::CIMNamespaceName("root/cimv2"),
                source
                                               );

        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Source"),
                            Pegasus::CIMValue(instance_source)
                        ));

        prop_ind = m_instance.findProperty("System");
        Pegasus::CIMValue value_target = m_instance.getProperty(prop_ind).getValue();
        Pegasus::CIMObjectPath target;
        value_target.get(target);

        Pegasus::CIMInstance instance_target = m_client->getInstance(
                Pegasus::CIMNamespaceName("root/cimv2"),
                target
                                               );

        in_param.append(Pegasus::CIMParamValue(
                            Pegasus::String("Target"),
                            Pegasus::CIMValue(instance_target)
                        ));

        m_client->invokeMethod(
            Pegasus::CIMNamespaceName("root/cimv2"),
            install_service.getPath(),
            Pegasus::CIMName("VerifyInstalledIdentity"),
            in_param,
            out_param);
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }
}

