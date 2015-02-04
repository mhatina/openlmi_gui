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

#include "logdialog.h"
#include "logger.h"
#include "settingsdialog.h"
#include "ui_logdialog.h"

#include <iostream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <sstream>

#define MAX_LINE_LENGTH 1024

void toLower(char *text)
{
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        text[i] = tolower(text[i]);
    }
}

QTreeWidgetItem *LogDialog::createItem(String name, QTreeWidgetItem *parent)
{
    Logger::getInstance()->debug("LogDialog::createItem(String name, QTreeWidgetItem *parent)");
    QTreeWidgetItem *item;
    if (parent == NULL) {
        item = new QTreeWidgetItem(m_ui->logs);
    } else {
        item = new QTreeWidgetItem(parent);
    }
    item->setText(0, name);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    m_ui->logs->sortByColumn(
        0,
        Qt::AscendingOrder
    );

    return item;
}

QTreeWidgetItem *LogDialog::findItem(String item_name, QTreeWidgetItem *parent)
{
    Logger::getInstance()->debug("LogDialog::topLevelNode(String item_name)");
    QList<QTreeWidgetItem *> list = m_ui->logs->findItems(item_name, Qt::MatchExactly | Qt::MatchRecursive);
    if (list.empty()) {
        return createItem(item_name, parent);
    } else {
        QTreeWidgetItem *item = NULL;
        for (int i = 0; i < list.size(); i++) {
            if (list[i]->parent() == parent) {
                item = list[i];
                break;
            }
        }

        if (item == NULL) {
            return createItem(item_name, parent);
        } else {
            return item;
        }
    }

    return NULL;
}

void LogDialog::initContextMenu()
{
    Logger::getInstance()->debug("LogDialog::initContextMenu()");
    m_context_menu = new QMenu(m_ui->logs);
    m_context_menu->setObjectName("log_context_menu");
    m_ui->logs->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *action = m_context_menu->addAction("Copy");
    action->setObjectName("copy_action");
    action->setEnabled(true);
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(copy()));
}

void LogDialog::initShowButton()
{
    QToolButton *button = m_ui->show_button;

    m_tool_button_menu = new QMenu();
    m_tool_button_menu->setObjectName("log_tool_button_menu");
    QAction *action = new QAction("All", m_tool_button_menu);
    action->setObjectName("show_all_action");
    button->setDefaultAction(action);
    m_tool_button_menu->addAction(action);

    action = new QAction("Info", m_tool_button_menu);
    action->setObjectName("show_info_action");
    m_tool_button_menu->addAction(action);

    action = new QAction("Debug", m_tool_button_menu);
    action->setObjectName("show_debug_action");
    m_tool_button_menu->addAction(action);

    action = new QAction("Error", m_tool_button_menu);
    action->setObjectName("show_error_action");
    m_tool_button_menu->addAction(action);

    action = new QAction("Critical", m_tool_button_menu);
    action->setObjectName("show_critical_action");
    m_tool_button_menu->addAction(action);

    button->setPopupMode(QToolButton::MenuButtonPopup);
    button->setMenu(m_tool_button_menu);
    button->setBackgroundRole(QPalette::Button);
}

void LogDialog::archive()
{
    QFile file(Logger::getInstance()->getLogPath());
    std::stringstream ss;
    ss << QDateTime().currentDateTime().date().day()
       << QDateTime().currentDateTime().date().month()
       << QDateTime().currentDateTime().date().year();

    String path = SettingsDialog::getInstance()->value<String, QLineEdit *>("log_archive");
    if (path.empty()) {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        path = dialog.getExistingDirectory();
        SettingsDialog::getInstance()->findChild<QLineEdit *>("log_archive")->setText(path);
    }
    path += "/lmicc_log_archive_" + ss.str();

    if (file.copy(path)) {
        Logger::getInstance()->info("Saved to " + path);
    } else {
        Logger::getInstance()->error(file.errorString());
    }
}

void LogDialog::changeShowOnly(QAction *action)
{
    int cnt = m_hidden.size();
    for (int i = cnt - 1; i >= 0; i--) {
        m_hidden[i]->setHidden(false);
        m_shown.push_back(m_hidden[i]);
        m_hidden.erase(m_hidden.begin() + i);
    }

    if (action->text() != "All") {
        cnt = m_shown.size();
        String show_only = "[" + action->text().toLower() + "]";
        for (int i = cnt - 1; i >= 0; i--) {
            String actual = m_shown[i]->text(0).toLower();
            if (actual.find(show_only) == String::npos) {
                m_shown[i]->setHidden(true);
                m_hidden.push_back(m_shown[i]);
                m_shown.erase(m_shown.begin() + i);
            }
        }
    }

    filterChanged(m_last_filter_text);
    m_ui->show_button->setText(action->text());
}

void LogDialog::copy()
{
    QList<QTreeWidgetItem *> list = m_ui->logs->selectedItems();
    if (list.empty())
        return;

    std::stringstream ss;
    int cnt = list.size();
    for (int i = 0; i < cnt; i++) {
        ss << copy(list[i]) << "\n";
    }
    qApp->clipboard()->setText(ss.str().c_str());
}

String LogDialog::copy(QTreeWidgetItem *child)
{
    int cnt = child->childCount();

    if (cnt != 0) {
        std::stringstream ss;
        for (int i = 0; i < cnt; i++) {
            ss << copy(child->child(i));
            if (i < cnt - 1)
                ss << "\n";
        }
        return ss.str();
    } else if (!child->isHidden()
               && (child->text(0).toStdString().find("[Info]") != String::npos
               || child->text(0).toStdString().find("[Debug]") != String::npos
               || child->text(0).toStdString().find("[Error]") != String::npos
               || child->text(0).toStdString().find("[Critical]") != String::npos)) {
        String text = child->text(0).toStdString().substr(1);
        text +=  "[" + child->parent()->text(0) + " " + text;
        return text;
    }

    return "";
}

void LogDialog::filterChanged(QString text)
{
    unsigned int cnt;
    String filter = text.toLower();
    if (((unsigned int) text.length()) >= m_last_filter_text.length()) {
        cnt = m_shown.size();
        for (int i = cnt - 1; i >= 0; i--) {
            String actual = m_shown[i]->text(0).toLower();
            if (actual.find(filter) == String::npos) {
                m_shown[i]->setHidden(true);
                m_hidden.push_back(m_shown[i]);
                m_shown.erase(m_shown.begin() + i);
            }
        }
    } else {
        cnt = m_hidden.size();
        String show_only = m_ui->show_button->text();
        if (show_only == "All") {
            show_only = "";
        }
        for (int i = cnt - 1; i >= 0; i--) {
            String actual = m_hidden[i]->text(0).toLower();
            if (actual.find(filter) != String::npos
                && actual.find(show_only) != String::npos) {
                m_hidden[i]->setHidden(false);
                m_shown.push_back(m_hidden[i]);
                m_hidden.erase(m_hidden.begin() + i);
            }
        }
    }

    m_last_filter_text = filter;
}

void LogDialog::showContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("LogDialog::showContextMenu(QPoint pos)");
    QPoint globalPos = m_ui->logs->mapToGlobal(pos);
    QTreeWidgetItem *item = m_ui->logs->itemAt(pos);

    findChild<QAction *>("copy_action")->setEnabled(item != NULL);
    m_context_menu->popup(globalPos);
}

LogDialog::LogDialog(QWidget *parent) :
    QDialog(parent),
    m_last_filter_text(""),
    m_ui(new Ui::LogDialog)
{
    m_ui->setupUi(this);
    initContextMenu();
    initShowButton();
    m_ui->filter->setFocus(Qt::ActiveWindowFocusReason);

    connect(
        m_ui->show_button,
        SIGNAL(triggered(QAction *)),
        this,
        SLOT(changeShowOnly(QAction *)));
    connect(
        m_ui->filter,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(filterChanged(QString)));
    connect(
        m_ui->logs,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showContextMenu(QPoint)));
    connect(
        m_ui->archive_button,
        SIGNAL(clicked()),
        this,
        SLOT(archive()));
}

LogDialog::~LogDialog()
{
    delete m_ui;
    delete m_tool_button_menu;
}

void LogDialog::setLogs()
{
    m_ui->logs->clear();

    QFile file(Logger::getInstance()->getLogPath());
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTreeWidgetItem *year_item = NULL;
    QTreeWidgetItem *month_item = NULL;
    QTreeWidgetItem *day_item = NULL;

    while (!file.atEnd()) {
        char line[MAX_LINE_LENGTH];
        file.readLine(line, MAX_LINE_LENGTH);
        int length = strlen(line);
        line[length - 1] = '\0'; // remove '\n'

        String str_line(line);

        int pos1 = 1;
        int pos2 = str_line.find(" ", pos1);
        String day = str_line.substr(pos1, pos2 - pos1);

        pos1 = pos2 + 1;
        pos2 = str_line.find(" ", pos1);
        String month = str_line.substr(pos1, pos2 - pos1);
        day += " " + month;

        pos1 = pos2 + 1;
        pos2 = str_line.find(" ", pos1);
        String year = str_line.substr(pos1, pos2 - pos1);
        month += " " + year;
        day += " " + year;

        if (year_item == NULL || year_item->text(0).toStdString().find(year) == String::npos) {
            year_item = findItem(year);
        }
        if (month_item == NULL || month_item->text(0).toStdString().find(month) == String::npos) {
            month_item = findItem(month, year_item);
        }
        if (day_item == NULL || day_item->text(0).toStdString().find(day) == String::npos) {
            day_item = findItem(day, month_item);
        }

        QTreeWidgetItem *log_entry = new QTreeWidgetItem(day_item);
        str_line = "[" + str_line.substr(pos2 + 1);
        log_entry->setText(0, str_line);
        log_entry->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        day_item->addChild(log_entry);
        m_shown.push_back(log_entry);
    }

    file.close();
}
