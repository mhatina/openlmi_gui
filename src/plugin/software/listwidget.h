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

#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QList>
#include <QListWidget>
#include <QWidget>

namespace Ui
{
class ListWidget;
}

class ListWidget : public QListWidget
{
    Q_OBJECT

private:
    Ui::ListWidget *m_ui;

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

public:
    explicit ListWidget(QWidget *parent = 0);
    ~ListWidget();

    QList<QListWidgetItem *> selectedItems() const;
    void addItem(QListWidgetItem *item);
    void removeItem(QListWidgetItem *item);

private slots:
    void doubleClicked(QListWidgetItem *item);
    void selectChanged();

signals:
    void getFocus();
    void itemDoubleClicked(QListWidgetItem *item);
    void itemSelectionChanged();
    void lostFocus();
};

#endif // LISTWIDGET_H
