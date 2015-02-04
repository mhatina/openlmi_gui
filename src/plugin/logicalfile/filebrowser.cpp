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

#include "filebrowser.h"
#include "filetree.h"

#include <QKeyEvent>

FileBrowser::FileBrowser(QWidget *parent) :
    QTreeWidget(parent)
{
    connect(
        this,
        SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
        this,
        SLOT(isDirEntered(QTreeWidgetItem *, int)));
}

void FileBrowser::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        && !selectedItems().empty()) {
        isDirEntered(selectedItems()[0], 0);
    } else {
        QTreeWidget::keyPressEvent(event);
    }
}

void FileBrowser::isDirEntered(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    Pegasus::CIMInstance dir = ((FileTree::Item *) item)->getCIMData();

    if (CIMValue::get_property_value(dir, "CreationClassName") == "LMI_UnixDirectory") {
        emit enterDir(item);
    } else if (CIMValue::get_property_value(dir, "CreationClassName") == "LMI_SymbolicLink") {
        emit enterSymlink(item);
    }
}
