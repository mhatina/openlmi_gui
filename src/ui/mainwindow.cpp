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

#include "config.h"
#include "mainwindow.h"
#include "showtextdialog.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QFile>
#include <QScrollBar>
#include <QSplitter>
#include <QTableWidget>
#include <QToolBar>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_toolbar(new QToolBar()),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    showMaximized();

    connect(
        m_ui->action_exit,
        SIGNAL(triggered()),
        this,
        SLOT(close()));
    connect(
        m_ui->action_show_log,
        SIGNAL(triggered()),
        this,
        SLOT(showLog()));
    connect(
        m_ui->action_exit,
        SIGNAL(triggered()),
        this,
        SLOT(closeAll()));
    connect(
        Logger::getInstance(),
        SIGNAL(showMessage(QString)),
        m_ui->status_bar,
        SLOT(showMessage(QString))
        );
    m_log_dialog.setTitle("Log");

    int cnt = sizeof(buttons) / sizeof(buttons[0]);
    for (int i = 0; i < cnt; i++) {
        if (strcmp(buttons[i].object_name, "") == 0) {
            m_toolbar->addSeparator();
            continue;
        }

        std::string path = ICON_PATH;
        path += buttons[i].icon_path;
        QPushButton *button = new QPushButton(QIcon(path.c_str()), "");
        button->setObjectName(buttons[i].object_name);
        button->setToolTip(buttons[i].tooltip);
        button->setShortcut(QKeySequence(buttons[i].shortcut));
        button->setEnabled(!buttons[i].disabled);
        button->setCheckable(buttons[i].checkable);
        m_toolbar->addWidget(button);
    }
    addToolBar(m_toolbar);
    m_ui->tree_widget->connectButtons(m_toolbar);
}

MainWindow::~MainWindow()
{
    delete m_ui;    
}

PCTreeWidget* MainWindow::getPcTreeWidget()
{
    return m_ui->tree_widget;
}

ProviderWidget* MainWindow::getProviderWidget()
{
    return m_ui->provider_widget;
}

QAction* MainWindow::getResetPasswdStorageAction()
{
    return m_ui->action_reset_password_storage;
}

QStatusBar* MainWindow::getStatusBar()
{
    return m_ui->status_bar;
}

QToolBar* MainWindow::getToolbar()
{
    return m_toolbar;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    closeAll();
}

void MainWindow::closeAll()
{
    qApp->closeAllWindows();
}

void MainWindow::showLog()
{
    QFile file(DEFAULT_LOG_PATH);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    m_log_dialog.setText(text.toStdString());
    m_log_dialog.show();
}
