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

#ifndef NEWGROUPDIALOG_H
#define NEWGROUPDIALOG_H

#include <QDialog>

namespace Ui
{
class NewGroupDialog;
}

/**
 * @brief The NewGroupDialog class
 *
 * Dialog for new group
 */
class NewGroupDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit NewGroupDialog(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~NewGroupDialog();

    /**
     * @brief Getter
     * @return true if group is system, else false
     */
    bool isSystemGroup();
    /**
     * @brief Getter
     * @return name of group
     */
    std::string getName();

private:
    Ui::NewGroupDialog *m_ui;
};

#endif // NEWGROUPDIALOG_H
