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

#ifndef LOGICALFILE_H
#define LOGICALFILE_H

#include "filetree.h"
#include "plugin.h"
#include "lmi_string.h"

#include <QtPlugin>

namespace Ui
{
class LogicalFilePlugin;
}

class LogicalFilePlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    FileTree m_tree;
    int m_file_cnt;
    Ui::LogicalFilePlugin *m_ui;

    FileTree::Item *emitFound(std::vector<void *> *data, FileTree::Item *parent);
    void getInfo(std::vector<void *> *data, FileTree::Item *parent);
    void getSymlinkInfo(std::vector<void *> *data, FileTree::Item *parent);
    Pegasus::CIMInstance *findDir(String name);

    QTreeWidget *getNotSelectedTree(QTreeWidgetItem *item);
    QTreeWidget *getSelectedTree();

    void populateTree(QTreeWidget *tree, std::vector<void *> *data, FileTree::Item *parent);

public:
    explicit LogicalFilePlugin();
    ~LogicalFilePlugin();
    virtual String getInstructionText();
    virtual String getLabel();
    virtual String getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

private slots:
    void changeCursor(QTreeWidgetItem *item, int column);
    void dirEntered(QTreeWidgetItem *item);
    void symlinkEntered(QTreeWidgetItem *item);
    void writeInfo(std::vector<void *> *data, FileTree::Item *parent);

signals:
    void writeData(std::vector<void *> *data, FileTree::Item *parent);

};

#endif // LOGICALFILE_H
