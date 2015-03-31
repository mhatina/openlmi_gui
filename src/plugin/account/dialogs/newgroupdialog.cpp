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

#include "newgroupdialog.h"
#include "ui_newgroupdialog.h"

NewGroupDialog::NewGroupDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::NewGroupDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    setWindowFlags(Qt::Popup);
}

NewGroupDialog::~NewGroupDialog()
{
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
}

std::string NewGroupDialog::getName()
{
    return m_ui->name_line->text().toStdString();
}

bool NewGroupDialog::isSystemGroup()
{
    return m_ui->system_group_box->checkState() == Qt::Checked;
}
