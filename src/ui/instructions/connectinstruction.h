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
 *  ***** END LICENSE BLOCK ***** */

#ifndef CONNECTINSTRUCTION_H
#define CONNECTINSTRUCTION_H

#include "instruction.h"
#include "cimclient.h"
#include "lmi_string.h"

/**
 * @brief The ConnectInstruction class
 *
 * Represent connection to host.
 */
class ConnectInstruction : public IInstruction
{
private:
    String m_hostname;
    String m_username;

public:
    /**
     * @brief Constructor
     * @param hostname
     * @param username
     */
    ConnectInstruction(const String &hostname, const String &username);
    IInstruction::Subject getSubject();
    String toString();
    void run();
};

#endif // CONNECTINSTRUCTION_H
