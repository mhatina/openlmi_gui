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

#include "account.h"
#include "dialogs/detailsdialog.h"
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
#include "cimvalue.h"
#include "memberbox.h"
#include "ui_account.h"

#include <fstream>
#include <sstream>
#include <Pegasus/Common/Array.h>
#include <QFileDialog>
#include <QMenu>

std::string AccountPlugin::convertNameToID(std::string name)
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

int AccountPlugin::findGroupIndex(std::string name)
{
    int group_cnt = m_group_table->rowCount();
    for (int i = 0; i < group_cnt; i++) {
        std::string tmp = m_group_table->item(i, 1)->text().toStdString();
        if (tmp == name) {
            return i;
        }
    }

    return -1;
}

void AccountPlugin::initContextMenu()
{
    Logger::getInstance()->debug("AccountPlugin::initContextMenu()");
    m_context_menu = new QMenu(this);
    m_context_menu->setObjectName("account_context_menu");
    setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *action = m_context_menu->addAction("Create new user/group");
    action->setObjectName("create_user_group_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(add()));

    action = m_context_menu->addAction("Delete user/group");
    action->setObjectName("delete_user_group_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(remove()));

    m_context_menu->addSeparator();

    action = m_context_menu->addAction("Show details");
    action->setObjectName("show_details_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showDetails()));

    m_context_menu->addSeparator();

    action = m_context_menu->addAction("Show/hide buttons");
    action->setObjectName("show_hide_buttons_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showButtons()));
}

void AccountPlugin::setSelectedLineColor(QList<QTableWidgetItem *>
        selectedItems, QColor color)
{
    for (int i = 0; i < selectedItems.size(); i++) {
        selectedItems[i]->setBackgroundColor(color);
    }
}

AccountPlugin::AccountPlugin() :
    IPlugin(),
    m_account_refreshed(false),
    m_do_not_clear(false),
    m_group_refreshed(false),
    m_last_group_name(""),
    m_last_user_name(""),
    m_ui(new Ui::AccountPlugin)
{
    m_ui->setupUi(this);
    m_user_table = findChild<QTableWidget *>("user_table");
    m_user_table->setShowGrid(false);
    m_group_table = findChild<QTableWidget *>("group_table");
    m_group_table->setShowGrid(false);

    initContextMenu();
    m_ui->add_button->setVisible(false);
    m_ui->remove_button->setVisible(false);

    int prop_cnt = sizeof(userHeaderItems) / sizeof(userHeaderItems[0]);
    m_user_table->setColumnCount(prop_cnt);
    for (int i = 0; i < prop_cnt; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(userHeaderItems[i].name);
        item->setToolTip(userHeaderItems[i].tooltip);
        m_user_table->setHorizontalHeaderItem(
            i,
            item
        );
    }

    prop_cnt = sizeof(groupHeaderItems) / sizeof(groupHeaderItems[0]);
    m_group_table->setColumnCount(prop_cnt);
    for (int i = 0; i < prop_cnt; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(groupHeaderItems[i].name);
        item->setToolTip(groupHeaderItems[i].tooltip);
        m_group_table->setHorizontalHeaderItem(
            i,
            item
        );
    }

    m_user_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_group_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(
        m_ui->add_button,
        SIGNAL(clicked()),
        this,
        SLOT(add()));
    connect(
        m_ui->remove_button,
        SIGNAL(clicked()),
        this,
        SLOT(remove()));
    connect(
        m_ui->user_table,
        SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
        this,
        SLOT(showDetails()));
    connect(
        m_ui->group_table,
        SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
        this,
        SLOT(showDetails()));
    connect(
        this,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showContextMenu(QPoint)));
    connect(
        m_ui->tab_widget,
        SIGNAL(currentChanged(int)),
        this,
        SLOT(currentTabChanged(int)));
    m_ui->filter_box->hide();
    setPluginEnabled(false);
}

AccountPlugin::~AccountPlugin()
{
    delete m_ui;
}

std::string AccountPlugin::getInstructionText()
{
    if (m_instructions.empty()) {
        return "";
    }

    std::stringstream account;
    std::stringstream group;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        if (m_instructions[i]->getSubject() == IInstruction::ACCOUNT) {
            if (account.str().empty()) {
                account << m_instructions[0]->toString();
            }
            account << m_instructions[i]->toString();
        } else if (m_instructions[i]->getSubject() == IInstruction::GROUP) {
            if (group.str().empty()) {
                group << m_instructions[0]->toString();
            }
            group << m_instructions[i]->toString();
        }
    }

    if (account.str().empty() || group.str().empty()) {
        return account.str() + group.str();
    }

    return account.str() + "\n" + group.str();
}

std::string AccountPlugin::getLabel()
{
    return "Account";
}

std::string AccountPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": " << m_user_instances.size() << " user(s)";
    if (!m_group_instances.empty()) {
        ss << ", " << m_group_instances.size() << " group(s) shown";
    } else {
        ss << " shown";
    }
    return ss.str();
}

void AccountPlugin::getData(std::vector<void *> *data)
{
    Logger::getInstance()->debug("AccountPlugin::getData(std::vector<void *> *data)");

    Pegasus::Array<Pegasus::CIMInstance> users;
    Pegasus::Array<Pegasus::CIMObject> users_obj;
    Pegasus::Array<Pegasus::CIMInstance> groups;

    std::string filter_user = m_ui->filter_user_line->text().toStdString();
    std::string filter_group = m_ui->filter_group_line->text().toStdString();
    try {
        if (!m_account_refreshed && m_ui->tab_widget->currentWidget()->children()[1] == m_user_table) {
            m_user_instances.clear();
            users = enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("LMI_Account"),
                        true,       // deep inheritance
                        false,      // local only
                        true,       // include qualifiers
                        false       // include class origin
                    );

            for (unsigned int i = 0; i < users.size(); i++) {
                Pegasus::CIMInstance *instance;
                if (!filter_user.empty()) {
                    instance = new Pegasus::CIMInstance(users[i]);
                    if (CIMValue::get_property_value(*instance, "Name").find(filter_user) == std::string::npos) {
                        continue;
                    }
                } else {
                    instance = new Pegasus::CIMInstance(users[i]);
                }

                data->push_back(instance);
                m_user_instances.push_back(*instance);
            }
            for (unsigned int i = 0; i < users.size(); i++) {
                std::string str_value = CIMValue::get_property_value(users[i], "Name");
                m_users.push_back(str_value);
            }
            m_account_refreshed = true;
        } else if (!m_group_refreshed && m_ui->tab_widget->currentWidget()->children()[1] == m_group_table) {
            m_group_instances.clear();
            groups = enumerateInstances(
                         Pegasus::CIMNamespaceName("root/cimv2"),
                         Pegasus::CIMName("LMI_Group"),
                         true,       // deep inheritance
                         false,      // local only
                         true,       // include qualifiers
                         false       // include class origin
                     );

            for (unsigned int i = 0; i < groups.size(); i++) {
                Pegasus::CIMInstance *instance;
                if (!filter_group.empty()) {
                    instance = new Pegasus::CIMInstance(groups[i]);
                    if (CIMValue::get_property_value(*instance, "Name").find(filter_group) == std::string::npos) {
                        continue;
                    }
                } else {
                    instance = new Pegasus::CIMInstance(groups[i]);
                }

                data->push_back(instance);
                m_group_instances.push_back(*instance);
            }
            data->push_back(new std::multimap<Pegasus::String, Pegasus::CIMInstance>());

            Pegasus::Array<Pegasus::CIMInstance> members;
            members = enumerateInstances(
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
                Pegasus::String collection = members[j].getProperty(
                                                 prop_ind).getValue().toString();
                ind = collection.reverseFind('=') + 2;
                collection = collection.subString(ind, collection.size() - ind - 1);

                users_obj = execQuery(
                                Pegasus::CIMNamespaceName("root/cimv2"),
                                Pegasus::String("WQL"),
                                Pegasus::String(std::string("SELECT * FROM LMI_Account WHERE UserID = \"" +
                                                CIMValue::to_std_string(member) + "\"").c_str())
                            );

                unsigned pos = data->size() - 1;
                ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[pos])->insert(
                    std::pair<Pegasus::String, Pegasus::CIMInstance>(collection,
                            Pegasus::CIMInstance(users_obj[0])));
            }
            m_group_refreshed = true;
        } else {
            return;
        }
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    emit doneFetchingData(data);
}

void AccountPlugin::clear()
{
    Logger::getInstance()->debug("AccountPlugin::clear()");

    if (m_do_not_clear) {
        m_do_not_clear = false;
        return;
    }

    m_changes_enabled = false;
    m_user_table->setRowCount(0);
    m_group_table->setRowCount(0);
    m_last_user_name = "";
    m_last_group_name = "";
    m_account_refreshed = false;
    m_group_refreshed = false;
    m_changes_enabled = true;
}

void AccountPlugin::fillTab(std::vector<void *> *data)
{
    Logger::getInstance()->debug("AccountPlugin::fillTab(std::vector<void *> *data)");
    m_changes_enabled = false;
    bool group = false;

    int prop_cnt;
    try {
        unsigned int cnt = data->size();
        if (cnt > 0) {
            Pegasus::CIMInstance *tmp = ((Pegasus::CIMInstance *) (*data)[0]);
            group = CIMValue::get_property_value(*tmp, "CreationClassName") == "LMI_Group";
        }
        for (unsigned int i = 0; i < cnt; i++) {
            if (group && i == cnt - 1) {
                break;
            }

            Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[i]);
            std::string creation_class_name = CIMValue::get_property_value(*instance, "CreationClassName");

            if (creation_class_name == "LMI_Account") {
                int row_count = m_user_table->rowCount();
                m_user_table->insertRow(row_count);
                prop_cnt = sizeof(userProperties) / sizeof(userProperties[0]);
                for (int k = 0; k < prop_cnt; k++) {
                    std::string str_value = CIMValue::get_property_value(*instance, userProperties[k].property);
                    QTableWidgetItem *item =
                        new QTableWidgetItem(str_value.c_str());
                    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                    item->setToolTip(str_value.c_str());
                    m_user_table->setItem(
                        row_count,
                        k,
                        item
                    );

                    if (!userProperties[k].enabled) {
                        m_user_table->item(row_count, k)->setFlags(item_flags);
                    }
                }
            } else if (creation_class_name == "LMI_Group") {


                int row_count = m_group_table->rowCount();
                std::string group_name;
                m_group_table->insertRow(row_count);
                prop_cnt = sizeof(groupProperties) / sizeof(groupProperties[0]);
                for (int k = 0; k < prop_cnt; k++) {
                    Pegasus::CIMProperty property;
                    std::string str_value = CIMValue::get_property_value(*instance, groupProperties[k].property, &property);
                    if (property.getName() == "Name") {
                        group_name = str_value;
                    }
                    QTableWidgetItem *item =
                        new QTableWidgetItem(str_value.c_str());
                    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                    item->setToolTip(str_value.c_str());
                    m_group_table->setItem(
                        row_count,
                        k,
                        item
                    );

                    if (!groupProperties[k].enabled) {
                        m_group_table->item(row_count, k)->setFlags(item_flags);
                    }
                }

                MemberBox *box = new MemberBox(group_name);
                connect(box, SIGNAL(add(std::string)), this, SLOT(addUserToGroup(std::string)));
                connect(box, SIGNAL(remove(std::string)), this, SLOT(removeUserFromGroup(std::string)));
                m_group_table->setCellWidget(
                    row_count,
                    prop_cnt,
                    box
                );

                std::multimap<Pegasus::String, Pegasus::CIMInstance> *members =
                    ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[cnt - 1]);
                std::pair <std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator,
                    std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator> ret =
                        members->equal_range(Pegasus::String(group_name.c_str()));

                for (std::multimap<Pegasus::String, Pegasus::CIMInstance>::iterator it =
                         ret.first; it != ret.second; it++) {
                    std::string user_name = CIMValue::get_property_value(it->second, "Name");

                    MemberBox *item = (MemberBox *) m_group_table->cellWidget(i, prop_cnt);
                    if (item != NULL) {
                        item->addItem(user_name);
                    }
                }
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }

    m_user_table->sortByColumn(0, Qt::AscendingOrder);
    m_group_table->sortByColumn(1, Qt::AscendingOrder);

    unsigned int cnt = data->size();
    for (unsigned int i = 0; i < cnt; i++) {
        delete ((Pegasus::CIMInstance *) (*data)[i]);
        if (group) {
            group = false;
            delete ((std::multimap<Pegasus::String, Pegasus::CIMInstance> *) (*data)[cnt - 1]);
            cnt--;
        }
    }

    m_changes_enabled = true;
}

void AccountPlugin::add()
{
    QTableWidget *current = (QTableWidget *)
                            m_ui->tab_widget->currentWidget()->children()[1];
    NewUserDialog *user_dialog = NULL;
    NewGroupDialog *group_dialog = NULL;
    std::string name;

    if (current == m_user_table) {
        user_dialog = new NewUserDialog(this);
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
        group_dialog = new NewGroupDialog(this);
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
    int row_count = current->rowCount();
    current->insertRow(row_count);
    for (int i = 0; i < current->columnCount(); i++) {
        QTableWidgetItem *item;
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
            item->setText("TBA");
            current->setItem(
                current->rowCount() - 1,
                i,
                item
            );
        }
    }
    current->selectRow(row_count);
    setSelectedLineColor(current->selectedItems(), Qt::green);
    m_changes_enabled = true;
}

void AccountPlugin::addUserToGroup(std::string group)
{
    std::vector<std::string> available_users;
    std::vector<std::string> selected_users;
    int cnt = m_users.size();
    int column = (sizeof(groupProperties) / sizeof(groupProperties[0]));
    int group_row = findGroupIndex(group);

    MemberBox *box = (MemberBox *) m_group_table->cellWidget(group_row, column);
    for (int i = 0; i < cnt; i++) {
        std::string user_name = m_users[i];
        if (box->findItem(user_name) == -1) {
            available_users.push_back(user_name);
        }
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
                           CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING,
                                   convertNameToID(selected_users[i]))
                       )
                      );
    }
}

void AccountPlugin::currentTabChanged(int i)
{
    Q_UNUSED(i)
    if (m_account_refreshed && m_group_refreshed) {
        return;
    }

    emit refreshProgress(Engine::NOT_REFRESHED, this);
    m_do_not_clear = true;
    refresh(m_client);
}

void AccountPlugin::remove()
{
    QTableWidget *current = (QTableWidget *)
                            m_ui->tab_widget->currentWidget()->children()[1];

    QModelIndexList tmp = current->selectionModel()->selectedIndexes();
    std::vector<int> rows;

    for (int i = 0; i < tmp.size(); i++) {
        rows.push_back(tmp[i].row());
    }

    std::sort(rows.begin(), rows.end());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());

    if (current == m_user_table) {
        for (int i = rows.size() - 1; i >= 0; i--) {
            m_last_user_name = m_user_table->item(rows[i], 0)->text().toStdString();
            m_user_table->selectRow(rows[i]);
            if (m_user_table->selectedItems()[0]->backgroundColor() != Qt::red) {
                setSelectedLineColor(m_user_table->selectedItems(), Qt::red);
            } else {
                setSelectedLineColor(m_user_table->selectedItems(), Qt::white);
                int pos = 0;
                while (1) {
                    pos = findInstruction(IInstruction::ACCOUNT, "delete_user", pos + 1);
                    if (((DeleteUserInstruction *) m_instructions[pos])->getUserName() ==
                        m_last_user_name) {
                        deleteInstruction(pos);
                        deleteInstruction(pos - 1);
                        break;
                    }
                }

                continue;
            }
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
            m_group_table->selectRow(rows[i]);
            if (m_group_table->selectedItems()[0]->backgroundColor() != Qt::red) {
                setSelectedLineColor(m_group_table->selectedItems(), Qt::red);
            } else {
                setSelectedLineColor(m_group_table->selectedItems(), Qt::white);
                int pos = 0;
                while (1) {
                    pos = findInstruction(IInstruction::GROUP, "delete_group", pos + 1);
                    if (((DeleteGroupInstruction *) m_instructions[pos])->getGroupName() ==
                        m_last_group_name) {
                        deleteInstruction(pos);
                        deleteInstruction(pos - 1);
                        break;
                    }
                }

                continue;
            }
            m_group_table->selectRow(rows[i]);
            setSelectedLineColor(m_group_table->selectedItems(), Qt::red);
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

void AccountPlugin::removeUserFromGroup(std::string group)
{
    std::vector<std::string> available_users;
    std::vector<std::string> selected_users;
    int column = (sizeof(groupProperties) / sizeof(groupProperties[0]));
    int group_row = findGroupIndex(group);

    MemberBox *box = (MemberBox *) m_group_table->cellWidget(group_row, column);
    for (int i = 0; i < box->itemCount(); i++) {
        available_users.push_back(box->getItem(i));
    }

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
                CIMValue::to_cim_value(Pegasus::CIMTYPE_STRING,
                                       convertNameToID(selected_users[i]))
            )
        );
    }
}

void AccountPlugin::showButtons()
{
    bool visible = m_ui->add_button->isVisible();
    m_ui->add_button->setVisible(!visible);
    m_ui->remove_button->setVisible(!visible);
}

void AccountPlugin::showContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("AccountPlugin::showContextMenu(QPoint pos)");
    QPoint globalPos = mapToGlobal(pos);
    m_context_menu->popup(globalPos);
}

void AccountPlugin::showDetails()
{
    QWidget *current = m_ui->tab_widget->currentWidget();
    QList<QTableWidgetItem *> selected_items;
    bool edited = false;

    if (current == m_ui->user_page) {
        DetailsDialog dialog("User details", this);
        Pegasus::CIMInstance *user = NULL;
        selected_items = m_ui->user_table->selectedItems();
        if (selected_items.empty()) {
            return;
        }
        std::string name_expected = selected_items[0]->text().toStdString();
        int cnt = m_user_instances.size();
        for (int i = 0; i < cnt; i++) {

            if (name_expected == CIMValue::get_property_value(m_user_instances[i],
                    "Name")) {
                user = &(m_user_instances[i]);
                break;
            }
        }

        if (user == NULL) {
            Logger::getInstance()->error("Unable to show details");
            return;
        }

        dialog.setValues(*user);

        int pos = 0;
        std::map<std::string, std::string> changes;
        while ((pos = findInstruction(IInstruction::ACCOUNT, "", pos)) != -1) {
            IInstruction *instruction = m_instructions[pos];
            std::string name = CIMValue::get_property_value(*user, "Name");
            std::string property_name = instruction->getInstructionName();
            std::string str_value = CIMValue::to_std_string(instruction->getValue());
            pos++;

            bool found = false;
            if (user->findProperty(Pegasus::CIMName(property_name.c_str())) !=
                Pegasus::Uint32(-1)) {
                found = ((ChangeUserPropertyInstruction *) instruction)->getUserName() == name;
            }

            if (!found) {
                continue;
            }

            changes[property_name] = str_value;
        }
        dialog.alterProperties(changes);

        if (dialog.exec()) {
            std::map<std::string, std::string> changes = dialog.getChanges();
            edited = !changes.empty();
            std::map<std::string, std::string>::iterator it;
            std::string name = CIMValue::get_property_value(*user, "Name");
            addInstruction(
                new GetInstruction(
                    IInstruction::ACCOUNT,
                    name
                )
            );
            for (it = changes.begin(); it != changes.end(); it++) {
                Pegasus::CIMProperty p;
                CIMValue::get_property_value(*user, it->first, &p);
                addInstruction
                (new ChangeUserPropertyInstruction(
                     m_client,
                     it->first,
                     name,
                     CIMValue::to_cim_value(p.getType(), it->second, p.isArray())
                 ));
            }
            addInstruction(
                new PushInstruction(
                    IInstruction::ACCOUNT
                )
            );
        }
    } else {
        DetailsDialog dialog("Group details", this);
        Pegasus::CIMInstance *group = NULL;
        selected_items = m_ui->group_table->selectedItems();
        if (selected_items.empty()) {
            return;
        }
        std::string name_expected = selected_items[1]->text().toStdString();
        int cnt = m_group_instances.size();
        for (int i = 0; i < cnt; i++) {

            if (name_expected == CIMValue::get_property_value(m_group_instances[i],
                    "Name")) {
                group = &(m_group_instances[i]);
            }
        }

        if (group == NULL) {
            Logger::getInstance()->error("Unable to show details");
            return;
        }

        dialog.setValues(*group);

        int pos = 0;
        std::map<std::string, std::string> changes;
        while ((pos = findInstruction(IInstruction::GROUP, "", pos)) != -1) {
            IInstruction *instruction = m_instructions[pos];
            std::string name = CIMValue::get_property_value(*group, "Name");
            std::string property_name = instruction->getInstructionName();
            std::string str_value = CIMValue::to_std_string(instruction->getValue());
            pos++;

            bool found = false;
            if (group->findProperty(Pegasus::CIMName(property_name.c_str())) !=
                Pegasus::Uint32(-1)) {
                found = ((ChangeGroupPropertyInstruction *) instruction)->getGroupName() ==
                        name;
            }

            if (!found) {
                continue;
            }

            changes[property_name] = str_value;
        }
        dialog.alterProperties(changes);

        if (dialog.exec()) {
            std::map<std::string, std::string> changes = dialog.getChanges();
            edited = !changes.empty();
            std::map<std::string, std::string>::iterator it;
            std::string name = CIMValue::get_property_value(*group, "Name");
            addInstruction(
                new GetInstruction(
                    IInstruction::GROUP,
                    name
                )
            );
            for (it = changes.begin(); it != changes.end(); it++) {
                Pegasus::CIMProperty p;
                CIMValue::get_property_value(*group, it->first, &p);
                addInstruction
                (new ChangeGroupPropertyInstruction(
                     m_client,
                     it->first,
                     name,
                     CIMValue::to_cim_value(p.getType(), it->second, p.isArray())
                 ));
            }
            addInstruction(
                new PushInstruction(
                    IInstruction::GROUP
                )
            );
        }
    }

    if (edited) {
        setSelectedLineColor(selected_items, Qt::yellow);
    }
}

Q_EXPORT_PLUGIN2(account, AccountPlugin)
