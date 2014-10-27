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

#ifndef LMIWBEM_CLIENT_H
#define LMIWBEM_CLIENT_H

#include <string>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/SSLContext.h>

/**
 * @brief The CIMClient class
 * @author Peter Hatina, Martin Hatina
 */
class CIMClient: public Pegasus::CIMClient
{
public:
    /**
     * @brief Constructor
     */
    CIMClient();

    /**
     * @brief Copy constructor
     * @param copy
     */
    CIMClient(const CIMClient &copy);

    /**
     * @brief Destructor
     */
    ~CIMClient();

    /**
     * @brief Connect to host
     * @param uri
     * @param username
     * @param password
     * @param trust_store
     */
    void connect(
        const std::string &uri,
        const std::string &username,
        const std::string &password,
        const std::string &trust_store = "/etc/pki/tls/certs");

    /**
     * @brief Connect to host
     * @param hostname
     * @param port
     * @param is_https
     * @param username
     * @param password
     * @param trust_store
     */
    void connect(
        const std::string &hostname,
        unsigned int port,
        bool is_https,
        const std::string &username,
        const std::string &password,
        const std::string &trust_store = "/etc/pki/tls/certs");

    /**
     * @brief Disconnect from host
     */
    void disconnect();

    /**
     * @brief Connection status
     * @return true if client is connected to host, else false
     */
    bool isConnected() const
    {
        return m_is_connected;
    }

    /**
     * @brief Setter
     * @param verify
     */
    void setVerifyCertificate(bool verify = true)
    {
        m_verify_cert = verify;
    }

    /**
     * @brief Getter
     * @return true if verification is turned on, else false
     */
    bool getVerifyCertificate() const
    {
        return m_verify_cert;
    }

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
     * @return username
     */
    std::string username() const
    {
        return m_username;
    }

private:
    // We hide these from Pegasus::CIMClient
    using Pegasus::CIMClient::connect;

    static Pegasus::Boolean dontVerifyCertificate(Pegasus::SSLCertificateInfo &ci);
    static Pegasus::Boolean verifyCertificate(Pegasus::SSLCertificateInfo &ci);

    std::string m_hostname;
    std::string m_username;
    bool m_is_connected;
    bool m_verify_cert;
};

#endif // LMIWBEM_CLIENT_H
