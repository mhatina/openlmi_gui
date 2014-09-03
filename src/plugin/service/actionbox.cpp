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

#include "actionbox.h"
#include "service.h"
#include "ui_actionbox.h"

ActionBox::ActionBox(std::string name) :
    QWidget(),
    m_name(name),
    m_ui(new Ui::ActionBox)
{
    m_ui->setupUi(this);
    int action_cnt = sizeof(action_list) / sizeof(action_list[0]);
    for (int i = 0; i < action_cnt; i++)
        m_ui->action_box->addItem(action_list[i]);

    connect(
        m_ui->action_box,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(currentChanged(int)));
}

ActionBox::~ActionBox()
{
    delete m_ui;
}

void ActionBox::currentChanged(int i)
{
    if (m_ui->action_box->currentText().toStdString() == action_list[0])
        return;

    emit performAction(m_name, (e_action) i);
}
