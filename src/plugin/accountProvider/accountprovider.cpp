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

#include "accountprovider.h"
#include "config.h"
#include "dialogs/groupmemberdialog.h"
#include "dialogs/newgroupdialog.h"
#include "dialogs/newuserdialog.h"
#include "instructions/addusertogroupinstruction.h"
#include "instructions/changegrouppropertyinstruction.h"
#include "instructions/changeuserpropertyinstruction.h"
#include "instructions/connectinstruction.h"
#include "instructions/deletegroupinstruction.h"
#include "instructions/deleteuserinstruction.h"
#include "instructions/getinstruction.h"
#include "instructions/newgroupinstruction.h"
#include "instructions/newuserinstruction.h"
#include "instructions/pushinstruction.h"
#include "instructions/removeuserfromgroupinstruction.h"
#include "lmiwbem_value.h"
#include "memberbox.h"
#include "ui_accountprovider.h"

#include <fstream>
#include <sstream>
#include <Pegasus/Common/Array.h>
#include <QFileDialog>

std::string AccountProviderPlugin::convertNameToID(std::string name)
{
    std::string id;

    for (int i = 0; i < m_user_table->rowCount(); i++) {
        if (m_user_table->item(i, 1)->text().toStdString() == name) {
            id = m_user_table->item(i, 0)->text().toStdString();
            break;
        }
    }
    return id;
}

int AccountProviderPlugin::findGroupIndex(std::string name)
{
    int group_cnt = m_group_table->rowCount();
    for (int i = 0; i < group_cnt; i++) {
        std::string tmp = m_group_table->item(i, 1)->text().toStdString();
        if (tmp == name)
            return i;
    }

    return -1;
}

AccountProviderPlugin::AccountProviderPlugin() :
    IPlugin(),        
    m_last_group_name(""),
    m_last_user_name(""),
    m_ui(new Ui::AccountProviderPlugin)
{
    m_ui->setupUi(this);
    m_user_table = findChild<QTableWidget*>("user_table");
    m_group_table = findChild<QTableWidget*>("group_table");

    int prop_cnt = sizeof(userProperties) / sizeof(userProperties[0]);
    m_user_table->setColumnCount(prop_cnt);
    for (int i = 0; i < prop_cnt; i++)
        m_user_table->setHorizontalHeaderItem(
                    i,
                    new QTableWidgetItem(userProperties[i].display_name)
                    );

    prop_cnt = sizeof(groupProperties) / sizeof(groupProperties[0]);
    m_group_table->setColumnCount(prop_cnt + 1);
    for (int i = 0; i < prop_cnt; i++)
        m_group_table->setHorizontalHeaderItem(
                    i,
                    new QTableWidgetItem(groupProperties[i].display_name)
                    );
    m_group_table->setHorizontalHeaderItem(
                prop_cnt,
                new QTableWidgetItem("Members")
                );

    m_user_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_group_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(
        m_user_table,
        SIGNAL(itemChanged(QTableWidgetItem*)),
        this,
        SLOT(itemChanged(QTableWidgetItem*))
        );
    connect(
        m_group_table,
        SIGNAL(itemChanged(QTableWidgetItem*)),
        this,
        SLOT(itemChanged(QTableWidgetItem*))
        );
    connect(
        m_ui->add_button,
        SIGNAL(clicked()),
        this,
        SLOT(add())
        );
    connect(
        m_ui->remove_button,
        SIGNAL(clicked()),
        this,
        SLOT(remove())
        );
    showFilter(false);
    setPluginEnabled(false);
}

AccountProviderPlugin::~AccountProviderPlugin()
{
    delete m_ui;
}

std::string AccountProviderPlugin::getInstructionText()
{
    if (m_instructions.empty())
        return "";

    std::stringstream account;
    std::stringstream group;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        if (m_instructions[i]->getSubject() == IInstruction::ACCOUNT) {
            if (account.str().empty())
                account << m_instructions[0]->toString();
            account << m_instructions[i]->toString();
        } else if (m_instructions[i]->getSubject() == IInstruction::GROUP) {
            if (group.str().empty())
                group << m_instructions[0]->toString();
            group << m_instructions[i]->toString();
        }
    }

    if (account.str().empty() || group.str().empty())
        return account.str() + group.str();

    return account.str() + "\n" + group.str();
}

std::string AccountProviderPlugin::getLabel()
{
    return "&Accounts";
}

void AccountProviderPlugin::generateCode()
{        
    if (!m_active)
        return;
    if (m_instructions.empty())
        return;

    std::ofstream user_out_file;
    std::ofstream group_out_file;
    std::string filename = m_save_dir_path + "/" + m_client->hostname();

    bool user_connected = false;
    bool group_connected = false;

    for (unsigned int i = 1; i < m_instructions.size(); i++) {
        switch (m_instructions[i]->getSubject()) {
        case IInstruction::ACCOUNT:
            if (!user_connected) {
                user_out_file.open(std::string(filename + "_users").c_str());
                user_out_file << m_instructions[0]->toString();
                user_connected = true;
            }

            user_out_file << m_instructions[i]->toString();
            break;
        case IInstruction::GROUP:
            if (!group_connected) {
                group_out_file.open(std::string(filename + "_groups").c_str());
                group_out_file << m_instructions[0]->toString();
                group_connected = true;
            }

            group_out_file << m_instructions[i]->toString();
            break;
        default:
            continue;
        }
    }

    if (user_out_file.is_open())
        user_out_file.close();
    if (group_out_file.is_open())
        group_out_file.close();
}

void AccountProviderPlugin::getData(std::vector<void *> *data)
{
    Logger::getInstance()->debug("getData");
    Pegasus::Array<Pegasus::CIMInstance> users;
    Pegasus::Array<Pegasus::CIMObject> users_obj;
    Pegasus::Array<Pegasus::CIMInstance> groups;
    Pegasus::Array<Pegasus::CIMObject> groups_obj;

    std::string filter_user = m_ui->filter_user_line->text().toStdString();
    std::string filter_group = m_ui->filter_group_line->text().toStdString();
    try {
        users = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_Account"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                    );
        if (!filter_user.empty()) {
            users_obj = m_client->execQuery(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::String("WQL"),
                        Pegasus::String(std::string("SELECT * FROM LMI_Account WHERE " + filter_user).c_str())
                        );
        }

        data->push_back(new std::vector<Pegasus::CIMInstance>());
        for (unsigned int i = 0; i < (filter_user.empty() ? users.size() : users_obj.size()); i++)
            ((std::vector<Pegasus::CIMInstance> *) (*data)[0])->push_back(filter_user.empty() ? users[i] : Pegasus::CIMInstance(users_obj[i]));
        for (unsigned int i = 0; i < users.size(); i++) {
            Pegasus::Uint32 propIndex = users[i].findProperty(Pegasus::CIMName("Name"));
            Pegasus::CIMProperty property = users[i].getProperty(propIndex);
            Pegasus::CIMValue value = property.getValue();
            std::string str_value = CIMValue::to_std_string(value);

            m_users.push_back(str_value);
        }

        if (filter_group.empty()) {
            groups = m_client->enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("LMI_Group"),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                        );
        } else {
            groups_obj = m_client->execQuery(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::String("WQL"),
                        Pegasus::String(std::string("SELECT * FROM LMI_Group WHERE " + filter_group).c_str())
                        );
        }
        data->push_back(new std::vector<Pegasus::CIMInstance>());
        for (unsigned int i = 0; i < (filter_group.empty() ? groups.size() : groups_obj.size()); i++)
            ((std::vector<Pegasus::CIMInstance> *) (*data)[1])->push_back(filter_group.empty() ? groups[i] : Pegasus::CIMInstance(groups_obj[i]));

        data->push_back(new std::multimap<Pegasus::String, Pegasus::CIMInstance>());

        Pegasus::Array<Pegasus::CIMInstance> members;
        members = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_MemberOfGroup"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                    );
        int members_cnt = members.size();
        for (int j = 0; j < members_cnt; j++) {
            int prop_ind = members[j].findProperty("Member");
            Pegasus::String member = members[j].getProperty(prop_ind).getValue().toString();
            Pegasus::Uint32 ind = member.reverseFind(':') + 1;
            member = member.subString(ind, member.size() - ind - 1);

            prop_ind = members[j].findProperty("Collection");
            Pegasus::String collection = members[j].getProperty(prop_ind).getValue().toString();
            ind = collection.reverseFind('=') + 2;
            collection = collection.subString(ind, collection.size() - ind - 1);

            users_obj = m_client->execQuery(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::String("WQL"),
                        Pegasus::String(std::string("SELECT * FROM LMI_Account WHERE UserID = \"" + std::string(member.getCString()) + "\"").c_str())
                        );

            ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[2])->insert(std::pair<Pegasus::String, Pegasus::CIMInstance>(collection, Pegasus::CIMInstance(users_obj[0])));
        }

    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }  

    emit doneFetchingData(data);
}

void AccountProviderPlugin::fillTab(std::vector<void *> *data)
{    
    Logger::getInstance()->debug("fillTab");
    m_changes_enabled = false;
    m_user_table->setRowCount(0);
    m_group_table->setRowCount(0);
    m_last_user_name = "";
    m_last_group_name = "";

    std::vector<Pegasus::CIMInstance> *users = ((std::vector<Pegasus::CIMInstance> *) (*data)[0]);
    std::vector<Pegasus::CIMInstance> *groups = ((std::vector<Pegasus::CIMInstance> *) (*data)[1]);
    std::multimap<Pegasus::String, Pegasus::CIMInstance> *members = ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[2]);

    int prop_cnt;
    try {
        Pegasus::Uint32 users_cnt = users->size();
        for (unsigned int j = 0; j < users_cnt; j++) {
            int row_count = m_user_table->rowCount();
            m_user_table->insertRow(row_count);
            prop_cnt = sizeof(userProperties) / sizeof(userProperties[0]);
            for (int k = 0; k < prop_cnt; k++) {
                Pegasus::Uint32 propIndex = (*users)[j].findProperty(Pegasus::CIMName(userProperties[k].property));
                if (propIndex == Pegasus::PEG_NOT_FOUND) {
                    Logger::getInstance()->error("property " + std::string(userProperties[k].property) + " not found");
                    continue;
                }

                Pegasus::CIMProperty property = (*users)[j].getProperty(propIndex);
                Pegasus::CIMValue value = property.getValue();
                std::string str_value = CIMValue::to_std_string(value);
                QTableWidgetItem *item =
                        new QTableWidgetItem(str_value.c_str());
                item->setToolTip(str_value.c_str());
                m_user_table->setItem(
                            row_count,
                            k,
                            item
                            );

                if (!userProperties[k].enabled)
                    m_user_table->item(row_count, k)->setFlags(item_flags);
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }

    try {
        Pegasus::Uint32 groups_cnt = groups->size();
        for (unsigned int j = 0; j < groups_cnt; j++) {
            int row_count = m_group_table->rowCount();
            std::string group_name;
            m_group_table->insertRow(row_count);
            prop_cnt = sizeof(groupProperties) / sizeof(groupProperties[0]);
            for (int k = 0; k < prop_cnt; k++) {
                Pegasus::Uint32 propIndex = (*groups)[j].findProperty(Pegasus::CIMName(groupProperties[k].property));
                if (propIndex == Pegasus::PEG_NOT_FOUND) {
                    Logger::getInstance()->error("property " + std::string(userProperties[k].property) + " not found");
                    continue;
                }

                Pegasus::CIMProperty property = (*groups)[j].getProperty(propIndex);
                Pegasus::CIMValue value = property.getValue();
                std::string str_value = CIMValue::to_std_string(value);
                if (property.getName() == "Name")
                    group_name = str_value;
                QTableWidgetItem *item =
                        new QTableWidgetItem(str_value.c_str());
                item->setToolTip(str_value.c_str());
                m_group_table->setItem(
                            row_count,
                            k,
                            item
                            );

                if (!groupProperties[k].enabled)
                    m_group_table->item(row_count, k)->setFlags(item_flags);
            }

            MemberBox *box = new MemberBox(group_name);
            connect(box, SIGNAL(add(std::string)), this, SLOT(addUserToGroup(std::string)));
            connect(box, SIGNAL(remove(std::string)), this, SLOT(removeUserFromGroup(std::string)));
            m_group_table->setCellWidget(
                        row_count,
                        prop_cnt,
                        box
                        );

            std::pair <std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator, std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator> ret;
            ret = members->equal_range(Pegasus::String(group_name.c_str()));
            for (std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator it = ret.first; it != ret.second; it++) {
                Pegasus::Uint32 propIndex = it->second.findProperty(Pegasus::CIMName("Name"));
                Pegasus::CIMProperty property = it->second.getProperty(propIndex);
                Pegasus::CIMValue value = property.getValue();
                std::string user_name = CIMValue::to_std_string(value);

                MemberBox *item = (MemberBox*) m_group_table->cellWidget(j, prop_cnt);
                if (item != NULL)
                    item->addItem(user_name);
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }

    delete ((std::vector<Pegasus::CIMInstance> *) (*data)[0]);
    delete ((std::vector<Pegasus::CIMInstance> *) (*data)[1]);
    delete ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[2]);

    m_changes_enabled = true;
}

void AccountProviderPlugin::add()
{
    QTableWidget *current = (QTableWidget*) m_ui->tab_widget->currentWidget()->children()[1];
    NewUserDialog *user_dialog = NULL;
    NewGroupDialog *group_dialog = NULL;
    std::string name;

    if (current == m_user_table) {
        user_dialog = new NewUserDialog();
        if (!user_dialog->exec()) {
            delete user_dialog;
            return;
        }

        name = user_dialog->getName();
        addInstruction(
                    new NewUserInstruction(
                        m_client,
                        user_dialog
                        )
                    );
    } else if (current == m_group_table) {
        group_dialog = new NewGroupDialog();
        if (!group_dialog->exec()) {
            delete group_dialog;
            return;
        }

        name = group_dialog->getName();
        addInstruction(
                    new NewGroupInstruction(
                        m_client,
                        group_dialog
                        )
                    );
    }

    m_changes_enabled = false;
    current->insertRow(current->rowCount());
    for (int i = 0; i < current->columnCount(); i++) {
        QTableWidgetItem* item;
        if (i == 1) {
            item = new QTableWidgetItem(
                QString(
                        name.c_str()
                    )
                );
            item->setFlags(item_flags);
            current->setItem(
                        current->rowCount() - 1,
                        i,
                        item
                        );
        } else {
            item = new QTableWidgetItem();
            item->setFlags(item_flags);
            current->setItem(
                        current->rowCount() - 1,
                        i,
                        item
                        );
        }
    }
    m_changes_enabled = true;
}

void AccountProviderPlugin::addUserToGroup(std::string group)
{
    std::vector<std::string> available_users;
    std::vector<std::string> selected_users;
    int cnt = m_users.size();
    int column = (sizeof(groupProperties) / sizeof(groupProperties[0]));
    int group_row = findGroupIndex(group);

    MemberBox *box = (MemberBox*) m_group_table->cellWidget(group_row, column);
    for (int i = 0; i < cnt; i++) {
        std::string user_name = m_users[i];
        if (box->findItem(user_name) == -1)
            available_users.push_back(user_name);
    }

    GroupMemberDialog::getUsers(available_users, selected_users);

    cnt = selected_users.size();
    for (int i = 0; i < cnt; i++) {
        box->addItem(selected_users[i]);
        addInstruction(
                    new GetInstruction(
                        IInstruction::GROUP,
                        group
                        )
                    );
        addInstruction(
                    new GetInstruction(
                        IInstruction::ACCOUNT,
                        selected_users[i],
                        IInstruction::GROUP
                        )
                    );
        addInstruction(new AddUserToGroupInstruction(
                           m_client,
                           group,
                           Pegasus::CIMValue(
                               Pegasus::String(
                                   convertNameToID(selected_users[i]).c_str()
                                   )
                               )
                           )
                       );
    }
}

void AccountProviderPlugin::itemChanged(QTableWidgetItem* item)
{
    if (!m_changes_enabled)
        return;

    QWidget *current = m_ui->tab_widget->currentWidget();
    item->setBackground(QBrush(QColor("yellow")));

    if (current == m_ui->user_page) {
        std::string user_name = m_user_table->item(item->row(), 1)->text().toStdString();
        if (m_last_user_name != user_name) {
            m_last_user_name = user_name;
            addInstruction(
                        new GetInstruction(
                            IInstruction::ACCOUNT,
                            m_last_user_name
                            )
                        );
            addInstruction(
                        new PushInstruction(
                            IInstruction::ACCOUNT
                            )
                        );
            m_user_push_pos = m_instructions.size() - 1;
        }

        ChangeUserPropertyInstruction *instruction = new ChangeUserPropertyInstruction(
                    m_client,
                    std::string(userProperties[item->column()].property),
                    m_last_user_name,
                    Pegasus::CIMValue(
                        Pegasus::String(
                            item->text().toStdString().c_str()
                        )
                    )
                );

        if (dynamic_cast<ChangeUserPropertyInstruction*>(m_instructions[m_user_push_pos - 1]) != NULL
                && ((ChangeUserPropertyInstruction*) m_instructions[m_user_push_pos - 1])->equals(instruction))
            delete instruction;
        else {
            insertInstruction(
                        instruction,
                        m_user_push_pos
                    );
            m_user_push_pos++;
        }
    } else if (current == m_ui->group_page) {
        std::string group_name = m_group_table->item(item->row(), 1)->text().toStdString();
        if (m_last_group_name != group_name) {
            m_last_group_name = group_name;
            addInstruction(
                        new GetInstruction(
                            IInstruction::GROUP,
                            m_last_group_name
                            )
                        );
            addInstruction(
                        new PushInstruction(
                            IInstruction::GROUP
                            )
                        );
            m_group_push_pos = m_instructions.size() - 1;
        }

        ChangeGroupPropertyInstruction *instruction = new ChangeGroupPropertyInstruction(
                    m_client,
                    std::string(groupProperties[item->column()].property),
                    m_last_group_name,
                    Pegasus::CIMValue(
                        Pegasus::String(
                            item->text().toStdString().c_str()
                        )
                    )
                );
        if (dynamic_cast<ChangeGroupPropertyInstruction*>(m_instructions[m_group_push_pos - 1]) != NULL
                && ((ChangeGroupPropertyInstruction*) m_instructions[m_group_push_pos - 1])->equals(instruction))
            delete instruction;
        else {
            insertInstruction(
                        instruction,
                        m_group_push_pos
                );
            m_group_push_pos++;
        }
    }
}

void AccountProviderPlugin::remove()
{
    QTableWidget *current = (QTableWidget*) m_ui->tab_widget->currentWidget()->children()[1];

    QModelIndexList tmp = current->selectionModel()->selectedIndexes();
    std::vector<int> rows;

    for (int i = 0; i < tmp.size(); i++)
        rows.push_back(tmp[i].row());

    std::sort(rows.begin(), rows.end());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());

    if (current == m_user_table) {
        for (int i = rows.size() - 1; i >= 0; i--) {
            m_last_user_name = m_user_table->item(rows[i], 1)->text().toStdString();
            m_user_table->removeRow(rows[i]);
            addInstruction(
                        new GetInstruction(
                               IInstruction::ACCOUNT,
                               m_last_user_name
                               )
                           );
            addInstruction(
                        new DeleteUserInstruction(
                            m_client,
                            m_last_user_name
                            )
                        );            
        }
        m_last_user_name = "invalid";
    } else if (current == m_group_table) {
        for (int i = rows.size() - 1; i >= 0; i--) {
            m_last_group_name = m_group_table->item(rows[i], 1)->text().toStdString();
            m_group_table->removeRow(rows[i]);
            addInstruction(
                        new GetInstruction(
                            IInstruction::GROUP,
                            m_last_group_name
                            )
                        );
            addInstruction(
                        new DeleteGroupInstruction(
                            m_client,
                            m_last_group_name
                            )
                        );            
        }
        m_last_group_name = "invalid";
    }
}

void AccountProviderPlugin::removeUserFromGroup(std::string group)
{
    std::vector<std::string> available_users;
    std::vector<std::string> selected_users;
    int column = (sizeof(groupProperties) / sizeof(groupProperties[0]));
    int group_row = findGroupIndex(group);

    MemberBox *box = (MemberBox*) m_group_table->cellWidget(group_row, column);
    for (int i = 0; i < box->itemCount(); i++)
        available_users.push_back(box->getItem(i));

    GroupMemberDialog::getUsers(available_users, selected_users);
    for (unsigned int i = 0; i < selected_users.size(); i++) {
        box->deleteItem(selected_users[i]);
        addInstruction(
                    new GetInstruction(
                        IInstruction::GROUP,
                        group
                        )
                    );
        addInstruction(
                    new GetInstruction(
                        IInstruction::ACCOUNT,
                        selected_users[i],
                        IInstruction::GROUP
                        )
                    );
        addInstruction(
                    new RemoveUserFromGroupInstruction(
                        m_client,
                        group,
                        Pegasus::CIMValue(
                            Pegasus::String(
                                convertNameToID(selected_users[i]).c_str()
                                )
                            )
                        )
                    );
    }
}

Q_EXPORT_PLUGIN2(accountProvider, AccountProviderPlugin)
