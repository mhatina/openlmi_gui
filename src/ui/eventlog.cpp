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

#include "eventlog.h"
#include "logger.h"

#include <boost/thread.hpp>

// TODO complete
void EventLog::checkEvents()
{
    while (1) {
        m_mutex->lock();
        if (m_end) {
            m_mutex->unlock();
            break;
        }
        m_mutex->unlock();

        // get event from systems
        QTreeWidgetItem *tmp;
        for (int i = 0; i < m_tree->topLevelItemCount(); i++) {
            tmp = m_tree->topLevelItem(i);
            if (tmp == NULL) {
                return;
            }

            int cnt = tmp->childCount();
            for (int j = 0; j < cnt; j++) {
                QTreeWidgetItem *child = tmp->child(j);
                std::string ip = child->text(0).toStdString();

                if (m_connections->find(ip) == m_connections->end()) {
                    emit silentConnection(ip);
                    child->setBackgroundColor(1, QColor(Qt::gray));
                    continue;
                }

                child->setBackgroundColor(1, QColor(Qt::green));
                CIMClient *client = m_connections->operator [](child->text(0).toStdString());
                client->enumerateInstances(
                            Pegasus::CIMNamespaceName("root/cimv2"),
                            Pegasus::CIMName("CIM_RecordLog"),
                            true,       // deep inheritance
                            false,      // local only
                            false,      // include qualifiers
                            false       // include class origin
                            );
            }
        }


        sleep(5);
    }
}

EventLog::EventLog() :
    m_end(false),
    m_connections(NULL),
    m_mutex(new QMutex()),
    m_tree(NULL)
{
}

EventLog::~EventLog()
{
    delete m_mutex;
}

void EventLog::end()
{
    m_mutex->lock();
    m_end = true;
    m_mutex->unlock();
    m_thread->join();
}

void EventLog::setConnectionStorage(connection_map *connections)
{
    m_connections = connections;
}

void EventLog::setPCTree(QTreeWidget *tree)
{
    m_tree = tree;
}

void EventLog::start()
{
    if (m_connections == NULL || m_tree == NULL) {
        Logger::getInstance()->error("Tree or connection storage not set");
        return;
    }

    m_thread = new boost::thread(boost::bind(&EventLog::checkEvents, this));
    m_thread->start_thread();
}
