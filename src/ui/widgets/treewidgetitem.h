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

#include "lmi_string.h"

#include <QTreeWidgetItem>

typedef enum {
    UNKNOWN,
    VALID,
    NOT_VALID
} validity;


class TreeWidgetItem : public QTreeWidgetItem
{
private:    
    String m_id;
    String m_ipv4;
    String m_ipv6;
    String m_name;
    String m_mac;
    validity m_valid;

public:
    explicit TreeWidgetItem(QTreeWidgetItem *parent = 0);
    TreeWidgetItem(QTreeWidget *view);

    String getIpv4() const;
    void setIpv4(const String &value);

    String getIpv6() const;
    void setIpv6(const String &value);

    String getName() const;
    void setName(const String &value);

    String getMac() const;
    void setMac(const String &mac);

    String getId() const;
    void setId(const String &id);

    validity getValid() const;
    void setValid(const validity &valid);
};

#endif // TREEWIDGETITEM_H
