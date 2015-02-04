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

#ifndef GROUPMEMBERDIALOG_H
#define GROUPMEMBERDIALOG_H

#include "lmi_string.h"

#include <QDialog>

namespace Ui
{
class GroupMemberDialog;
}

/**
 * @brief The GroupMemberDialog class
 *
 * Show possible users for addition/deletion to/from group
 */
class GroupMemberDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit GroupMemberDialog(QWidget *parent = 0);
    ~GroupMemberDialog();

    /**
     * @brief Get selected users
     * @param available_users -- possible users for addition/deletion to/from group
     * @param selected -- selected users
     *
     * Show dialog with available users.
     */
    static void getUsers(std::vector<String> &available_users,
                         std::vector<String> &selected);

private:
    Ui::GroupMemberDialog *m_ui;
};

#endif // GROUPMEMBERDIALOG_H
