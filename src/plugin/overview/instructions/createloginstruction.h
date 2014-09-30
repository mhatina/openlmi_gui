#ifndef CREATELOGINSTRUCTION_H
#define CREATELOGINSTRUCTION_H

#include "instructions/instruction.h"
#include "lmiwbem_client.h"

class CreateLogInstruction : public IInstruction
{
private:
    CIMClient *m_client;
    Pegasus::Uint16 m_severity;
    std::string m_message;

public:
    CreateLogInstruction(CIMClient *client, std::string message,
                         Pegasus::Uint16 severity);

    virtual Subject getSubject();
    virtual std::string toString();
    virtual void run();
};

#endif // CREATELOGINSTRUCTION_H
