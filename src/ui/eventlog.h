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

#ifndef EVENTLOG_H
#define EVENTLOG_H

#include "lmiwbem_client.h"

#include <boost/thread.hpp>
#include <map>
#include <string>
#include <QMutex>
#include <QTreeWidget>

class EventLog : QObject
{
    Q_OBJECT

    typedef std::map<std::string, CIMClient*> connection_map;

private:
    bool m_end;
    boost::thread *m_thread;
    connection_map *m_connections;
    QMutex *m_mutex;
    QTreeWidget *m_tree;

    void checkEvents();

public:
    EventLog();
    ~EventLog();
    void end();
    void setConnectionStorage(connection_map *connections);
    void setPCTree(QTreeWidget *tree);
    void start();

signals:
    void silentConnection(std::string ip);
};

#endif // EVENTLOG_H
