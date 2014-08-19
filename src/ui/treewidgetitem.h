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

#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>

class TreeWidgetItem : public QTreeWidgetItem
{
private:
    std::string m_ipv4;
    std::string m_ipv6;
    std::string m_name;
    std::string m_mac;

public:
    explicit TreeWidgetItem(QTreeWidgetItem *parent = 0);
    TreeWidgetItem(QTreeWidget *view);

    std::string getIpv4() const;
    void setIpv4(const std::string &value);

    std::string getIpv6() const;
    void setIpv6(const std::string &value);

    std::string getName() const;
    void setName(const std::string &value);

    std::string getMac() const;
    void setMac(const std::string &mac);
};

#endif // TREEWIDGETITEM_H
