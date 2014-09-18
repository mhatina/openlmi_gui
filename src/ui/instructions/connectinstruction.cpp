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

#include "connectinstruction.h"
#include "logger.h"

#include <sstream>

ConnectInstruction::ConnectInstruction(const std::string &hostname,
                                       const std::string &username) :
    IInstruction("connect"),
    m_hostname(hostname),
    m_username(username)
{
    Logger::getInstance()->debug("ConnectInstruction::ConnectInstruction(const std::string &hostname, const std::string &username)");
}

IInstruction::Subject ConnectInstruction::getSubject()
{
    Logger::getInstance()->debug("ConnectInstruction::getSubject()");
    return IInstruction::CONNECT;
}

std::string ConnectInstruction::toString()
{
    Logger::getInstance()->debug("ConnectInstruction::toString()");
    std::stringstream ss;
    ss << "c = connect(\"" << m_hostname << "\", \"" << m_username << "\")\n";
    return ss.str();
}

void ConnectInstruction::run()
{
    Logger::getInstance()->debug("ConnectInstruction::run()");
}
