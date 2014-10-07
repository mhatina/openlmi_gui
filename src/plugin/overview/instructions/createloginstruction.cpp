#include "createloginstruction.h"
#include "logger.h"

#include <sstream>

CreateLogInstruction::CreateLogInstruction(CIMClient *client,
        std::string message, Pegasus::Uint16 severity) :
    IInstruction("create_log_instruction"),
    m_client(client),
    m_severity(severity),
    m_message(message)
{
}

IInstruction::Subject CreateLogInstruction::getSubject()
{
    return IInstruction::OVERVIEW;
}

std::string CreateLogInstruction::toString()
{
    std::stringstream ss;
    ss << "c.root.cimv2.LMI_JournalLogRecord.create_instance({\"CreationClassName\": \"LMI_JournalLogRecord\",\n"
       << "\t\t\"LogCreationClassName\": \"LMI_JournalMessageLog\",\n"
       << "\t\t\"LogName\": \"Journal\",\n"
       << "\t\t\"DataFormat\": \"" + m_message + "\"})\n";
    return ss.str();
}

void CreateLogInstruction::run()
{
    try {
        Pegasus::CIMInstance instance(Pegasus::CIMName("LMI_JournalLogRecord"));

        Pegasus::CIMProperty log_creation_class(
            Pegasus::CIMName("LogCreationClassName"),
            Pegasus::CIMValue(
                Pegasus::String(
                    "LMI_JournalMessageLog")
            ));
        instance.addProperty(log_creation_class);

        Pegasus::CIMProperty log_name(
            Pegasus::CIMName("LogName"),
            Pegasus::CIMValue(
                Pegasus::String(
                    "Journal")
            ));
        instance.addProperty(log_name);

        Pegasus::CIMProperty data_format(
            Pegasus::CIMName("DataFormat"),
            Pegasus::CIMValue(
                Pegasus::String(
                    m_message.c_str()
                )
            ));
        instance.addProperty(data_format);

        if (m_severity > 7) {
            m_severity = 0;
        }
        Pegasus::CIMProperty severity(
            Pegasus::CIMName("PerceivedSeverity"),
            Pegasus::CIMValue(
                Pegasus::Uint16(
                    m_severity
                )
            ));
        instance.addProperty(severity);

        // BUG not working, wrong type
        m_client->createInstance(
            Pegasus::CIMNamespaceName("root/cimv2"),
            instance);
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }
}
