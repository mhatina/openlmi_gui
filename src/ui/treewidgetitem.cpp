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

#include "treewidgetitem.h"

TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    m_ipv4(""),
    m_ipv6(""),
    m_name(""),
    m_mac("")
{
}

TreeWidgetItem::TreeWidgetItem(QTreeWidget *view) :
    QTreeWidgetItem(view)
{
}

std::string TreeWidgetItem::getIpv4() const
{
    return m_ipv4;
}

void TreeWidgetItem::setIpv4(const std::string &value)
{
    m_ipv4 = value;
}

std::string TreeWidgetItem::getIpv6() const
{
    return m_ipv6;
}

void TreeWidgetItem::setIpv6(const std::string &value)
{
    m_ipv6 = value;
}

std::string TreeWidgetItem::getName() const
{
    return m_name;
}

void TreeWidgetItem::setName(const std::string &value)
{
    m_name = value;
}

std::string TreeWidgetItem::getMac() const
{
    return m_mac;
}

void TreeWidgetItem::setMac(const std::string &mac)
{
    m_mac = mac;
}
std::string TreeWidgetItem::getId() const
{
    return m_id;
}

void TreeWidgetItem::setId(const std::string &id)
{
    m_id = id;
}
