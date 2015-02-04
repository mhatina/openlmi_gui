#ifndef CREATELOGINSTRUCTION_H
#define CREATELOGINSTRUCTION_H

#include "instructions/instruction.h"
#include "cimclient.h"
#include "lmi_string.h"

class CreateLogInstruction : public IInstruction
{
private:
    CIMClient *m_client;
    Pegasus::Uint16 m_severity;
    String m_message;

public:
    CreateLogInstruction(CIMClient *client, String message,
                         Pegasus::Uint16 severity);

    virtual Subject getSubject();
    virtual String toString();
    virtual void run();
};

#endif // CREATELOGINSTRUCTION_H
