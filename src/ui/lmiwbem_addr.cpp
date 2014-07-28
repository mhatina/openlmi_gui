/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Peter Hatina <phatina@redhat.com>,
 *                            Martin Hatina <mhatina@redhat.com>
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

#include <cerrno>
#include <cstdlib>
#include "lmiwbem_addr.h"
#include "logger.h"

Address::Address(std::string url)
    : m_hostname("unknown")
    , m_port(5989)
    , m_is_https(true)
    , m_is_valid(true)
{
    Logger::getInstance()->debug("Address::Address(std::string url)");
    if (url.find("http://") != std::string::npos) {
        url.erase(0, 7);
        m_is_https = false;
    } else if (url.find("https://") != std::string::npos) {
        url.erase(0, 8);
    }

    size_t pos = url.rfind(':');
    if (pos != std::string::npos) {
        m_hostname = url.substr(0, pos);
        long int port = strtol(url.substr(pos + 1,
            url.size() - pos - 1).c_str(), NULL, 10);
        if (errno == ERANGE || port < 0 || port > 65535)
            m_is_valid = false;
        m_port = static_cast<unsigned int>(port);
    } else {
        m_hostname = url;
    }
}
