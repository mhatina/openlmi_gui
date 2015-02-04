/* ***** BEGIN LICENSE BLOCK *****
 *
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 2.1 of the
 *   License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301 USA
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef FILETREE_H
#define FILETREE_H

#include "cimclient.h"
#include "cimvalue.h"
#include "lmi_string.h"

#include <Pegasus/Common/CIMInstance.h>
#include <QTreeWidgetItem>

class FileTree
{
public:
    class Item;

    class Item : public QTreeWidgetItem
    {
        friend class FileTree;
        friend struct item_equals;

    private:
        Pegasus::CIMInstance *m_data;
        Item *m_parent;
        std::vector<Item> m_descendant;

    public:
        Item() :
            QTreeWidgetItem(),
            m_data(NULL),
            m_parent(NULL)
        {}
        Item(Item *parent) :
            QTreeWidgetItem(),
            m_data(NULL),
            m_parent(parent)
        {}
        Item(const Item &copy) :
            QTreeWidgetItem(copy.parent()),
            m_data(copy.m_data),
            m_parent(copy.m_parent)
        {
            unsigned int cnt = copy.m_descendant.size();
            for (unsigned int i = 0; i < cnt; i++) {
                m_descendant.push_back(copy.m_descendant[i]);
            }
        }

        void addDescendant(Item descendant)
        {
            if (std::find(m_descendant.begin(), m_descendant.end(), descendant) == m_descendant.end())
                m_descendant.push_back(descendant);
        }

        unsigned int count() { return m_descendant.size(); }
        Item &getDescendant(unsigned int index) { return m_descendant[index]; }

        virtual Pegasus::CIMInstance getCIMData() { return *m_data; }
        void setCIMData(Pegasus::CIMInstance *data) { m_data = data; }

        String getName() { return text(0); }
        String getFullName() { return CIMValue::get_property_value(getCIMData(), "Name"); }

        Item *getParent() { return m_parent; }

        bool operator ==(Item item)
        {
            if (m_data == NULL || item.m_data == NULL)
                return false;
            return m_data->identical(*item.m_data);
        }

        bool operator !=(Item item)
        {
            return !(operator ==(item));
        }
    };

    class RootItem : public Item
    {
    private:
        CIMClient *m_client;
        Pegasus::CIMValue m_value;
    public:
        RootItem(CIMClient *client) :
            Item(NULL),
            m_client(client),
            m_value()
        {}

        virtual Pegasus::CIMInstance getCIMData()
        {
            if (m_value.isNull()) {
                Pegasus::Uint32 ind = m_data->findProperty("PartComponent");
                Pegasus::CIMProperty property = m_data->getProperty(ind);
                m_value = property.getValue();
            }

            Pegasus::CIMObjectPath path;
            m_value.get(path);
            Pegasus::CIMInstance root =
                m_client->getInstance(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    path,
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                );
            root.setPath(Pegasus::CIMObjectPath(path));
            return root;
        }
    };

    class SymbolicLink : public Item
    {
    private:
        bool link_to_dir;

    public:
        SymbolicLink(Item *parent) :
            Item(parent),
            link_to_dir(false)
        {}

        String getTargetName() { return CIMValue::get_property_value(getCIMData(), "TargetFile"); }
    };


private:
    RootItem m_root;

public:
    FileTree();

    Item &getRoot() { return m_root; }
    void setRoot(RootItem &root) { m_root = root; }

    Item *findItem(String path, Item *root_item = NULL);
};

#endif // FILETREE_H
