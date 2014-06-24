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

#ifndef NEWUSERDIALOG_H
#define NEWUSERDIALOG_H

#include <QDialog>

namespace Ui {
class NewUserDialog;
}

/**
 * @brief The NewUserDialog class
 *
 * Dialog for new user
 */
class NewUserDialog : public QDialog
{
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit NewUserDialog(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~NewUserDialog();

    /**
     * @brief Getter
     * @return true if group should be created, else false
     */
    bool createGroup();
    /**
     * @brief Getter
     * @return true if home should be created, else false
     */
    bool createHome();
    /**
     * @brief Getter
     * @return true if user account is system, else false
     */
    bool isSystemAccount();
    /**
     * @brief Getter
     * @return description of user
     */
    std::string getGecos();
    /**
     * @brief Getter
     * @return home directory of user
     */
    std::string getHomeDir();
    /**
     * @brief Getter
     * @return name of user
     */
    std::string getName();
    /**
     * @brief Getter
     * @return password
     */
    std::string getPasswd();
    /**
     * @brief Getter
     * @return what shell user uses
     */
    std::string getShell();
    
private:
    Ui::NewUserDialog *m_ui;

private slots:
    void changeHomeDir(QString text);
};

#endif // NEWUSERDIALOG_H
