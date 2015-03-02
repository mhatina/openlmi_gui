/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Dominika Hoďovská <dominika.hodovska@gmail.com>
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

#include "menubartestsuite.h"

MenuBarTestSuite::MenuBarTestSuite() :
    AbstractTestSuite()
{
}

/*
 * TODO class for logs, test ctrl+L etc.
 * what for is field in logs window
 */
void MenuBarTestSuite::actionAboutTestResult() {

    QList<QMessageBox *> childList = getWindow<QMessageBox *>();
    if (childList.isEmpty())
        qDebug("Failed to get QMessageBox about_dialog");
    else if (childList.size() != 1)
        qDebug("More QMessageBoxes found");

    QMessageBox *detail = kernel->widget<QMessageBox *>("about_dialog");
    detail = childList.at(0);
    QVERIFY2(detail->isVisible(), "about_dialog is not active after action_about->trigger()");

    QTest::keyClick(detail, Qt::Key_Escape);
}

//void MenuBarTestSuite::testOptionsAction() {

//}

//ready?
void MenuBarTestSuite::testCloseApp()
{
    QSKIP("Ready.", SkipSingle);
    QAction *exit = kernel->widget<QAction *>("action_exit");
    QVERIFY2(exit, "Failed to get action_exit Widget");
    QVERIFY2(exit->isVisible(), "Exit menu item is not visible");

    QVERIFY2(!h->getClosed(), "Main window is not visible");
    exit->trigger();
    QVERIFY2(h->getClosed(), "Main window is visible");
}

//ready?
void MenuBarTestSuite::testReloadPluginsAction()
{
    QSKIP("Ready.", SkipSingle);
    QTabWidget *plugins = kernel->widget<QTabWidget *>("tab_widget");
    QVERIFY2(plugins, "Failed to get tab_widget");
    QAction *reloadPluginsAct = kernel->widget<QAction *>("action_reload_plugins");
    QVERIFY2(reloadPluginsAct, "Failed to get action_reload_plugins widget");

    QVERIFY2(plugins->count() > 0, "No plugins loaded at start");

    kernel->deletePlugins();
    QVERIFY2(plugins->count() == 0, "Plugins found after delete");

    reloadPluginsAct->trigger();
    QVERIFY2(plugins->count() > 0, "No plugins loaded after reload");
}

//ready?
void MenuBarTestSuite::testShowAbout()
{
    QSKIP("Ready", SkipSingle);
    QAction *actionAbout = kernel->widget<QAction *>("action_about");
    QVERIFY2(actionAbout, "Failed to get action_about widget");

    QMessageBox *detail = kernel->widget<QMessageBox *>("about_dialog");
    QVERIFY2(detail == NULL, "about_dialog not null before action_about triggering");

    boost::thread t(boost::bind(&MenuBarTestSuite::actionAboutTestResult,this));
    actionAbout->trigger();
    QTest::qWait(1000);
    t.join();
}
