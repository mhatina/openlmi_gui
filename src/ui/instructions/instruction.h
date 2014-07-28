/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <Pegasus/Common/CIMValue.h>
#include <QObject>

/**
 * @brief The Instruction interface
 *
 * Represent one instruction (change made in provider)
 */
class IInstruction : public QObject
{
    Q_OBJECT

protected:
    Pegasus::CIMValue m_value;
    std::string m_instruction;

public:

    /**
     * Enum of related subject
     */
    typedef enum
    {
        UNKNOWN,
        CONNECT,
        ACCOUNT,
        GROUP,
        SERVICE,
        SOFTWARE
    } Subject;

    /**
     * @brief Constructor
     * @param instruction -- name of instruction
     * @param value -- value of change
     */
    IInstruction(std::string instruction, Pegasus::CIMValue value);
    /**
     * @brief Constructor
     * @param instruction -- name of instruction
     */
    IInstruction(std::string instruction);
    Pegasus::CIMValue getValue();
    std::string getInstructionName();
    /**
     * @brief Virtual destructor
     */
    virtual ~IInstruction();
    /**
     * @brief Getter
     * @return subject
     */
    virtual Subject getSubject() = 0;
    /**
     * @brief toString
     * @return LMIShell code representation of instruction
     */
    virtual std::string toString() = 0;
    /**
     * @brief apply the change
     */
    virtual void run() = 0;
};

#endif // INSTRUCTION_H
