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

#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H

#include "ui_authenticationdialog.h"

#include <QDialog>

namespace Ui
{
class AuthenticationDialog;
}

/**
 * @brief The AuthenticationDialog class
 *
 * Dialog for user authentication.
 */
class AuthenticationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Contructor
     * @param id -- id of host (ip or domain name)
     */
    AuthenticationDialog(std::string id);

    /**
     * @brief Destructor
     */
    ~AuthenticationDialog();

    /**
     * @brief Getter
     * @return password
     */
    std::string getPasswd();

    /**
     * @brief Getter
     * @return username
     */
    std::string getUsername();

private:
    Ui::AuthenticationDialog *m_ui;
};

#endif // AUTHENTICATIONDIALOG_H
