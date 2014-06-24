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

#ifndef ACCOUNTPROVIDERPLUGIN_H
#define ACCOUNTPROVIDERPLUGIN_H

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
    {"ID", "UserID", false},
    {"Name", "Name", false},
    {"Gecos", "ElementName", true},
    {"System name", "SystemName", false},
    {"Home directory", "HomeDirectory", true},
    {"Login shell", "LoginShell", true},
    {"Last login", "LastLogin", false}
};

const Qt::ItemFlags item_flags =
        Qt::ItemIsSelectable |
        Qt::ItemIsEnabled;

namespace Ui {
class AccountProviderPlugin;
}

/**
 * @brief The AccountProviderPlugin class
 */
class AccountProviderPlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    int m_group_push_pos;
    int m_user_push_pos;
    QTableWidget *m_group_table;
    QTableWidget *m_user_table;
    std::string m_last_group_name;
    std::string m_last_user_name;
    std::vector<std::string> m_users;
    Ui::AccountProviderPlugin *m_ui;

    int findGroupIndex(std::string name);
    std::string convertNameToID(std::string name);

public:
    explicit AccountProviderPlugin();
    ~AccountProviderPlugin();
    virtual std::string getInstructionText();
    virtual std::string getLabel();    
    virtual void fillTab(std::vector<void *> *data);
    virtual void generateCode();
    virtual void getData(std::vector<void *> *data);

private slots:    
    void add();
    void addUserToGroup(std::string group);
    void itemChanged(QTableWidgetItem* item);
    void remove();
    void removeUserFromGroup(std::string group);
};

#endif // ACCOUNTPROVIDERPLUGIN_H
