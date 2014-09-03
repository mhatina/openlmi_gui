#ifndef SOFTWAREINSTRUCTION_H
#define SOFTWAREINSTRUCTION_H

#include "instructions/instruction.h"
#include "lmiwbem_client.h"

class SoftwareInstruction : public IInstruction
{
protected:
    CIMClient *m_client;
    Pegasus::CIMInstance m_instance;
    std::string m_name;

public:
    SoftwareInstruction(CIMClient *client, std::string instruction, Pegasus::CIMInstance instance);

    Pegasus::CIMValue invokeInstallMethod(Pegasus::Uint16 install_option);
    std::string getName();
};

#endif // SOFTWAREINSTRUCTION_H
