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

#include "listwidget.h"
#include "ui_listwidget.h"

ListWidget::ListWidget(QWidget *parent) :
    QListWidget(parent),
    m_ui(new Ui::ListWidget)
{
    m_ui->setupUi(this);
    connect(
        m_ui->listWidget,
        SIGNAL(itemDoubleClicked(QListWidgetItem *)),
        this,
        SLOT(doubleClicked(QListWidgetItem *)));
    connect(
        m_ui->listWidget,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(selectChanged()));
}

ListWidget::~ListWidget()
{
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
}

QList<QListWidgetItem *> ListWidget::selectedItems() const
{
    return m_ui->listWidget->selectedItems();
}

void ListWidget::addItem(QListWidgetItem *item)
{
    m_ui->listWidget->addItem(item);
}

void ListWidget::removeItem(QListWidgetItem *item)
{
    m_ui->listWidget->removeItemWidget(item);
}

#include <iostream>

void ListWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    emit getFocus();
}

void ListWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    emit lostFocus();
}

void ListWidget::doubleClicked(QListWidgetItem *item)
{
    emit itemDoubleClicked(item);
}

void ListWidget::selectChanged()
{
    emit itemSelectionChanged();
}
