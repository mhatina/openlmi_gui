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

#include <QKeyEvent>

TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    m_ipv4(""),
    m_ipv6(""),
    m_name(""),
    m_mac(""),
    m_valid(UNKNOWN)
{
}

TreeWidgetItem::TreeWidgetItem(QTreeWidget *view) :
    QTreeWidgetItem(view)
{
}

String TreeWidgetItem::getIpv4() const
{
    return m_ipv4;
}

void TreeWidgetItem::setIpv4(const String &value)
{
    m_ipv4 = value;
}

String TreeWidgetItem::getIpv6() const
{
    return m_ipv6;
}

void TreeWidgetItem::setIpv6(const String &value)
{
    m_ipv6 = value;
}

String TreeWidgetItem::getName() const
{
    return m_name;
}

void TreeWidgetItem::setName(const String &value)
{
    m_name = value;
}

String TreeWidgetItem::getMac() const
{
    return m_mac;
}

void TreeWidgetItem::setMac(const String &mac)
{
    m_mac = mac;
}
String TreeWidgetItem::getId() const
{
    return m_id;
}

void TreeWidgetItem::setId(const String &id)
{
    m_id = id;
}

validity TreeWidgetItem::getValid() const
{
    return m_valid;
}

void TreeWidgetItem::setValid(const validity &valid)
{
    m_valid = valid;
}
