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

#ifndef DISCOVERWORKER_H
#define DISCOVERWORKER_H

#include <iostream>
#include <list>
#include <QObject>
#include <slp.h>
#include <string>

/**
 * @brief The DiscoverWorker class
 *
 * Discover hosts with tog-pegasus service.
 */
class DiscoverWorker : public QObject
{
    Q_OBJECT

private:   
    std::string m_pc_lang;
    bool m_async;
    SLPHandle m_hslp;
    SLPError m_errcode;
    
public:    
    /**
     * @brief Constructor
     */
    DiscoverWorker();    
    /**
     * @brief Close SLP
     */
    void close();
    /**
     * @brief Open SLP
     */
    void open();

public slots:    
    /**
     * @brief Discover hosts
     */
    void discover();

signals:
    /**
     * @brief Represent end of discovering
     * @param pc -- all discovered hosts
     */
    void finished(std::list<std::string> *pc);    

};

#endif // DISCOVERWORKER_H
