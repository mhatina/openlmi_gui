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

#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include "lmi_string.h"

#include <QClipboard>
#include <QDialog>
#include <QFile>
#include <QTreeWidgetItem>

namespace Ui
{
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

private:
    String m_last_filter_text;
    std::vector<QTreeWidgetItem *> m_hidden;
    std::vector<QTreeWidgetItem *> m_shown;
    QMenu *m_context_menu;
    QMenu *m_tool_button_menu;
    Ui::LogDialog *m_ui;

    QTreeWidgetItem *createItem(String name, QTreeWidgetItem *parent);
    QTreeWidgetItem *findItem(String item_name, QTreeWidgetItem *parent = NULL);
    void initContextMenu();
    void initShowButton();

private slots:
    String copy(QTreeWidgetItem *child);
    void archive();
    void changeShowOnly(QAction *action);
    void copy();    
    void filterChanged(QString text);
    void showContextMenu(QPoint pos);

public:
    explicit LogDialog(QWidget *parent = 0);
    ~LogDialog();

    void setLogs();
};

#endif // LOGDIALOG_H
