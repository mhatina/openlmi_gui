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

#include "mainwindow.h"
#include "showtextdialog.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QFile>
#include <QMenu>
#include <QScrollBar>
#include <QSplitter>
#include <QTableWidget>
#include <QToolBar>
#include <QToolButton>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_toolbar(new QToolBar()),
    m_ui(new Ui::MainWindow)
{
    Logger::getInstance()->setParent(this);
    Logger::getInstance()->debug("MainWindow::MainWindow(QWidget *parent)");
    m_ui->setupUi(this);

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

        std::string path = buttons[i].icon_path;
        QPushButton *button;
        if (!path.empty())
            button = new QPushButton(QIcon(QPixmap(path.c_str())), "");
        else
            button = new QPushButton(buttons[i].tooltip);
        button->setObjectName(buttons[i].object_name);
        button->setToolTip(buttons[i].tooltip);
        button->setShortcut(QKeySequence(buttons[i].shortcut));
        button->setEnabled(!buttons[i].disabled);
        button->setCheckable(buttons[i].checkable);
        button->setFlat(true);
        m_toolbar->addWidget(button);
    }

    QToolButton *button = new QToolButton(m_toolbar);

    QMenu *menu = new QMenu();
    QAction *action = new QAction(QIcon(":/reboot.png"), "Reboot", menu);
    action->setObjectName("reboot_action");
    action->setIconVisibleInMenu(true);
    button->setDefaultAction(action);
    menu->addAction(action);

    action = new QAction(QIcon(":/shutdown.png"), "Shutdown", menu);
    action->setObjectName("shutdown_action");
    action->setIconVisibleInMenu(true);
    menu->addAction(action);

    action = new QAction(QIcon(":/reboot.png"), "Force reset", menu);
    action->setObjectName("force_reset_action");
    action->setIconVisibleInMenu(true);
    menu->addAction(action);

    action = new QAction(QIcon(":/shutdown.png"), "Force off", menu);
    action->setObjectName("force_off_action");
    action->setIconVisibleInMenu(true);
    menu->addAction(action);

    action = new QAction(QIcon(":/shutdown.png"), "Wake on Lan", menu);
    action->setObjectName("wake_on_lan");
    action->setIconVisibleInMenu(true);
    menu->addAction(action);

    button->setObjectName("power_button");
    button->setPopupMode(QToolButton::MenuButtonPopup);
    button->setMenu(menu);
    button->setBackgroundRole(QPalette::Button);
    button->setEnabled(false);
    m_toolbar->insertWidget(m_toolbar->actions()[12], button); // after apply button

    addToolBar(m_toolbar);
    m_ui->tree_widget->connectButtons(m_toolbar);
}

MainWindow::~MainWindow()
{
    Logger::getInstance()->debug("MainWindow::~MainWindow()");
    delete m_ui;    
}

PCTreeWidget* MainWindow::getPcTreeWidget()
{
    Logger::getInstance()->debug("MainWindow::getPcTreeWidget()");
    return m_ui->tree_widget;
}

ProviderWidget* MainWindow::getProviderWidget()
{
    Logger::getInstance()->debug("MainWindow::getProviderWidget()");
    return m_ui->provider_widget;
}

QAction* MainWindow::getResetPasswdStorageAction()
{
    Logger::getInstance()->debug("MainWindow::getResetPasswdStorageAction()");
    return m_ui->action_reset_password_storage;
}

QStatusBar* MainWindow::getStatusBar()
{
    Logger::getInstance()->debug("MainWindow::getStatusBar()");
    return m_ui->status_bar;
}

QToolBar* MainWindow::getToolbar()
{
    Logger::getInstance()->debug("MainWindow::getToolbar()");
    return m_toolbar;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Logger::getInstance()->debug("MainWindow::closeEvent(QCloseEvent *event)");
    Q_UNUSED(event);

    closeAll();
}

void MainWindow::closeAll()
{
    Logger::getInstance()->debug("MainWindow::closeAll()");
    qApp->closeAllWindows();
}

void MainWindow::showLog()
{
    Logger::getInstance()->debug("MainWindow::showLog()");
    QFile file(Logger::getInstance()->getLogPath().c_str());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    m_log_dialog.setText(text.toStdString());
    m_log_dialog.show();
}
