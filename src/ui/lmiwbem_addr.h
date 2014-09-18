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

#ifndef WBEM_CLIENT_ADDRESS_H
#define WBEM_CLIENT_ADDRESS_H

#include <string>

/**
 * @brief The Address class
 * @author Peter Hatina, Martin Hatina
 */
class Address
{
public:
    /**
     * @brief Constructor
     * @param url -- text representation of url
     */
    Address(std::string url);

    /**
     * @brief Getter
     * @return hostname
     */
    std::string hostname() const
    {
        return m_hostname;
    }

    /**
     * @brief Getter
     * @return port
     */
    unsigned int port() const
    {
        return m_port;
    }

    /**
     * @brief Check if https protocol is used
     * @return true if https is used, else false
     */
    bool isHttps() const
    {
        return m_is_https;
    }

    /**
     * @brief Check address validity
     * @return if address is valid return true, else false
     */
    bool isValid() const
    {
        return m_is_valid;
    }

private:
    std::string m_hostname;
    unsigned int m_port;
    bool m_is_https;
    bool m_is_valid;
};

#endif // WBEM_CLIENT_ADDRESS_H
