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

#include "authenticationdialog.h"

AuthenticationDialog::AuthenticationDialog(std::string id):
    m_ui(new Ui::AuthenticationDialog)
{
    m_ui->setupUi(this);
    m_ui->id_label->setText(id.c_str());
    m_ui->passwd_line->setEchoMode(QLineEdit::Password);
    connect(m_ui->ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

AuthenticationDialog::~AuthenticationDialog()
{
}

std::string AuthenticationDialog::getPasswd()
{
    return m_ui->passwd_line->text().toStdString();
}

std::string AuthenticationDialog::getUsername()
{
    return m_ui->username_line->text().toStdString();
}
