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

#include "discoverworker.h"
#include "logger.h"

#include <cstring>
#include <iostream>
#include <list>
#include <slp.h>

/**
 * @brief get_srvs_list
 * @param hslp
 * @param srvurl
 * @param lifetime
 * @param errcode
 * @param cookie
 * @return
 */
SLPBoolean get_srvs_list (
    SLPHandle hslp,
    const char *srvurl,
    unsigned short lifetime,
    SLPError errcode,
    void *cookie )
{
    Logger::getInstance()->debug("get_srvs_list (SLPHandle hslp, const char* srvurl, unsigned short lifetime, SLPError errcode, void* cookie )");
    if ((errcode == SLP_OK || errcode == SLP_LAST_CALL) && srvurl != NULL) {
        SLPSrvURL *url;
        SLPParseSrvURL(srvurl, &url);

        ((std::list<std::string> *)cookie)->push_back(url->s_pcHost);
        SLPFree(url);
    }

    return SLP_TRUE;
}

DiscoverWorker::DiscoverWorker() :
    m_pc_lang(""),
    m_async(false),
    m_errcode(SLP_OK)
{
    Logger::getInstance()->debug("DiscoverWorker::DiscoverWorker()");
}

void DiscoverWorker::close()
{
    Logger::getInstance()->debug("DiscoverWorker::close()");
    SLPClose(m_hslp);
}

void DiscoverWorker::open()
{
    Logger::getInstance()->debug("DiscoverWorker::open()");
    m_errcode = SLPOpen(m_pc_lang.c_str(), m_async ? SLP_TRUE : SLP_FALSE, &m_hslp);
}

void DiscoverWorker::discover()
{
    Logger::getInstance()->debug("DiscoverWorker::discover()");
    open();
    std::list<std::string> *pc = new std::list<std::string>();

    m_errcode = SLPFindSrvs(
                    m_hslp,
                    "wbem",
                    0,
                    0,
                    get_srvs_list,
                    pc
                );

    close();
    pc->sort();

    emit finished(pc);
}
