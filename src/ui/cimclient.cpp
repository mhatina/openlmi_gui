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

#include <Pegasus/Common/SSLContext.h>
#include "cimclient.h"
#include "cimaddress.h"
#include "logger.h"
#include "kernel.h"

CIMClient::CIMClient()
    : Pegasus::CIMClient()
    , m_hostname()
    , m_username()
    , m_is_connected(false)
    , m_verify_cert(true)
{
    Logger::getInstance()->debug("CIMClient::CIMClient()");
}

CIMClient::CIMClient(const CIMClient &copy) :
    Pegasus::CIMClient(),
    m_hostname(copy.m_hostname),
    m_username(copy.m_username),
    m_is_connected(copy.m_is_connected),
    m_verify_cert(copy.m_verify_cert)
{
    Logger::getInstance()->debug("CIMClient::CIMClient(const CIMClient &copy)");
}

CIMClient::~CIMClient()
{
    Logger::getInstance()->debug("CIMClient::~CIMClient()");
    disconnect();
}

void CIMClient::connect(
    const std::string &uri,
    const std::string &username,
    const std::string &password,
    const std::string &trust_store)
{
    Logger::getInstance()->debug("CIMClient::connect(const std::string &uri, const std::string &username, const std::string &password, const std::string &trust_store)");
    Address addr(uri);
    if (!addr.isValid()) {
        Logger::getInstance()->error("Invalid host address!");
        return;
    }

    connect(addr.hostname(), addr.port(), addr.isHttps(), username, password,
            trust_store);
}

void CIMClient::connect(
    const std::string &hostname,
    unsigned int port,
    bool is_https,
    const std::string &username,
    const std::string &password,
    const std::string &trust_store)
{
    Logger::getInstance()->debug("CIMClient::connect(const std::string &hostname, unsigned int port, bool is_https, const std::string &username, const std::string &password, const std::string &trust_store)");
    m_hostname = hostname;
    m_username = username;
    if (!is_https) {
        Pegasus::CIMClient::connect(
            Pegasus::String(hostname.c_str()),
            Pegasus::Uint32(port),
            Pegasus::String(username.c_str()),
            Pegasus::String(password.c_str()));
    } else {
        Pegasus::Boolean (*verify_cb)(Pegasus::SSLCertificateInfo & ci) =
            m_verify_cert ? verifyCertificate : dontVerifyCertificate;
        Pegasus::SSLContext ctx(
            Pegasus::String(trust_store.c_str()),
            verify_cb);
        Pegasus::CIMClient::connect(
            Pegasus::String(hostname.c_str()),
            Pegasus::Uint32(port),
            ctx,
            Pegasus::String(username.c_str()),
            Pegasus::String(password.c_str()));
    }
    m_is_connected = true;
}

void CIMClient::disconnect()
{
    Logger::getInstance()->debug("CIMClient::disconnect()");
    Pegasus::CIMClient::disconnect();
    m_hostname.clear();
    m_is_connected = false;
}

Pegasus::Boolean CIMClient::dontVerifyCertificate(Pegasus::SSLCertificateInfo
        &ci)
{
    UNUSED(ci)
    Logger::getInstance()->debug("CIMClient::dontVerifyCertificate(Pegasus::SSLCertificateInfo &ci)");
    return true;
}

Pegasus::Boolean CIMClient::verifyCertificate(Pegasus::SSLCertificateInfo &ci)
{
    Logger::getInstance()->debug("CIMClient::verifyCertificate(Pegasus::SSLCertificateInfo &ci)");
    return static_cast<Pegasus::Boolean>(ci.getResponseCode());
}
