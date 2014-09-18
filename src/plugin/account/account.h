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

#ifndef ACCOUNTPLUGIN_H
#define ACCOUNTPLUGIN_H

#include "plugin.h"

#include <QTableWidget>
#include <QtPlugin>
#include <QWidget>

const Engine::property groupProperties[] = {
    {"ID", "InstanceID", false},
    {"Name", "Name", false},
    {"Common Name", "CommonName", false},
    {"Element Name", "ElementName", false}
};

const Engine::property userProperties[] = {
    {"Name", "Name", false},
    {"Full Name", "ElementName", false},
    {"UID", "UserID", false},
    {"Home directory", "HomeDirectory", false},
    {"Login shell", "LoginShell", true}
};

const Qt::ItemFlags item_flags =
    Qt::ItemIsSelectable |
    Qt::ItemIsEnabled;

namespace Ui
{
class AccountPlugin;
}

/**
 * @brief The AccountPlugin class
 */
class AccountPlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    QTableWidget *m_group_table;
    QTableWidget *m_user_table;
    std::string m_last_group_name;
    std::string m_last_user_name;
    std::vector<Pegasus::CIMInstance> m_group_instances;
    std::vector<Pegasus::CIMInstance> m_user_instances;
    std::vector<std::string> m_users;
    Ui::AccountPlugin *m_ui;

    bool isKeyProperty(const char *property);
    int findGroupIndex(std::string name);
    std::string convertNameToID(std::string name);
    void setSelectedLineColor(QList<QTableWidgetItem *> selectedItems,
                              QColor color);

public:
    explicit AccountPlugin();
    ~AccountPlugin();
    virtual std::string getInstructionText();
    virtual std::string getLabel();
    virtual std::string getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

private slots:
    void add();
    void addUserToGroup(std::string group);
    void remove();
    void removeUserFromGroup(std::string group);
    void showDetails();
};

#endif // ACCOUNTPLUGIN_H
