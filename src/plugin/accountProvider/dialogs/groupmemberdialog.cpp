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

#include "groupmemberdialog.h"
#include "ui_groupmemberdialog.h"

#include <vector>

GroupMemberDialog::GroupMemberDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::GroupMemberDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

GroupMemberDialog::~GroupMemberDialog()
{
    delete m_ui;
}

void GroupMemberDialog::getUsers(std::vector<std::string> &available_users, std::vector<std::string> &selected)
{
    GroupMemberDialog dialog;
    for (unsigned int i = 0; i < available_users.size(); i++) {
        dialog.m_ui->list->addItem(available_users[i].c_str());
    }


    if (dialog.exec()) {
        QList<QListWidgetItem*> list = dialog.m_ui->list->selectedItems();
        for (int i = 0; i < list.size(); i++)
            selected.push_back(list[i]->text().toStdString());
    }
}
