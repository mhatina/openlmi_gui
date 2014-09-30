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

#include "newuserdialog.h"
#include "ui_newuserdialog.h"

NewUserDialog::NewUserDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NewUserDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_ui->name_line, SIGNAL(textChanged(QString)), this,
            SLOT(changeHomeDir(QString)));
    m_ui->name_line->setFocus();
    setWindowFlags(Qt::Popup);
}

NewUserDialog::~NewUserDialog()
{
    delete m_ui;
}

bool NewUserDialog::createGroup()
{
    return m_ui->create_group_box->checkState() == Qt::Checked;
}

bool NewUserDialog::createHome()
{
    return m_ui->create_home_box->checkState() == Qt::Checked;
}

bool NewUserDialog::isSystemAccount()
{
    return m_ui->system_account_box->checkState() == Qt::Checked;
}

std::string NewUserDialog::getGecos()
{
    return m_ui->gecos_line->text().toStdString();
}

std::string NewUserDialog::getHomeDir()
{
    return m_ui->home_dir_line->text().toStdString();
}

std::string NewUserDialog::getName()
{
    return m_ui->name_line->text().toStdString();
}

std::string NewUserDialog::getPasswd()
{
    return m_ui->password_line->text().toStdString();
}

std::string NewUserDialog::getShell()
{
    return m_ui->shell_line->text().toStdString();
}

void NewUserDialog::changeHomeDir(QString text)
{
    m_ui->home_dir_line->setText("/home/" + text);
}
