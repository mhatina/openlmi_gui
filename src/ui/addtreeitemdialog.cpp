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

#include "addtreeitemdialog.h"
#include "ui_addtreeitemdialog.h"

#include <QPushButton>

AddTreeItemDialog::AddTreeItemDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AddTreeItemDialog)
{
    m_ui->setupUi(this);
    m_ui->button_box->button(QDialogButtonBox::Ok)->setEnabled(false);
    m_ui->name->setCursorPosition(0);

    setWindowFlags(Qt::Popup);
    m_ui->name->setFocus(Qt::PopupFocusReason);

    move(
        parent->mapToGlobal(parent->pos()).rx() + 15,
        parent->mapToGlobal(parent->rect().center()).ry() - 150);

    connect(
        m_ui->name,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(checkName(QString)));
}

AddTreeItemDialog::~AddTreeItemDialog()
{
    m_ui->name->clearFocus();
    delete m_ui;    
}

std::string AddTreeItemDialog::getName()
{
    return m_ui->name->text().toStdString();
}

void AddTreeItemDialog::checkName(QString text)
{
    bool empty = text.size() == 0;
    m_ui->button_box->button(QDialogButtonBox::Ok)->setEnabled(!empty);

    if (empty) {
        m_ui->name->setStyleSheet("QLineEdit{background:red;}");
    } else {
        m_ui->name->setStyleSheet("QLineEdit{background:white;}");
    }
}
