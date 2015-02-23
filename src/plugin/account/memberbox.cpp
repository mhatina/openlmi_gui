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

#include "memberbox.h"
#include "ui_memberbox.h"

MemberBox::MemberBox(String owner) :
    m_ui(new Ui::MemberBox),
    m_owner(owner)
{
    m_ui->setupUi(this);
    connect(m_ui->add_button, SIGNAL(clicked()), this, SLOT(addUser()));
    connect(m_ui->remove_button, SIGNAL(clicked()), this, SLOT(removeUser()));
}

MemberBox::~MemberBox()
{
    delete m_ui;
}

int MemberBox::findItem(String text)
{
    return m_ui->member_box->findText(text.c_str());
}

int MemberBox::itemCount()
{
    return m_ui->member_box->count();
}

String MemberBox::getItem(int i)
{
    return m_ui->member_box->itemText(i).toStdString();
}

void MemberBox::addItem(String text)
{
    m_ui->member_box->addItem(text.c_str());
}

void MemberBox::deleteItem(String text)
{
    m_ui->member_box->removeItem(findItem(text));
}

void MemberBox::addUser()
{
    emit add(m_owner);
}

void MemberBox::removeUser()
{
    emit remove(m_owner);
}
