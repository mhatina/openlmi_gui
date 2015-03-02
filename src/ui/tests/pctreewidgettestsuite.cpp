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

#include "pctreewidgettestsuite.h"

#include <QMenu>
#include <QMenuBar>

PCTreeWidgetTestSuite::PCTreeWidgetTestSuite() :
    AbstractTestSuite()
{
}

void PCTreeWidgetTestSuite::addGroup(std::string g) {

    std::vector<QTreeWidgetItem*> foundGroups;

    QAction* addGroupsAction = kernel->widget<QAction*>("create_group_action");
    QVERIFY2(addGroupsAction, "Failed to get create_group_action");

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    boost::thread t(boost::bind(&QAction::trigger,addGroupsAction));
    QTest::qWait(1000);

    foundGroups = findGroup("");
    if (foundGroups.size() != 1) {
        QFAIL("Can not find recently added group which name hasn't been set yet.");
    }
    foundGroups.at(0)->setText(0, g.c_str());
    t.join();
}

void PCTreeWidgetTestSuite::deleteGroup(std::string g) {
    QAction* deleteGroupAction = kernel->widget<QAction*>("delete_group_action");
    QVERIFY2(deleteGroupAction, "Failed to get delete_group_action");
    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    std::vector<QTreeWidgetItem*> foundGroups = findGroup(g);
    if (foundGroups.size() == 0) {
        std::stringstream ss;
        ss << "Failed to find group named " << g;
        QFAIL(ss.str().c_str());
    }

    QPoint point = tree->visualItemRect(foundGroups.at(0)).center();
    kernel->getMainWindow()->getPcTreeWidget()->showContextMenu(point);
    QTest::qWait(1000);
    if (!deleteGroupAction->isEnabled())
        std::cerr << g.c_str() << std::endl;
    QVERIFY2(deleteGroupAction->isEnabled(), "delete_group_action not enabled after rightclicking on group.");
    deleteGroupAction->trigger();

    QTest::qWait(100);
    foundGroups = findGroup(g);
    QVERIFY2(foundGroups.size() == 0, "Failed to remove group.");
}

void PCTreeWidgetTestSuite::testAddedGroupPresence() {

    QSKIP("Not developed now", SkipSingle);
    std::vector<QTreeWidgetItem*> foundGroups;
    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after app start.");

    deleteGroup("Added");
    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 0, "Failed to delete Added group.");

    cleanup();
    init();

    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after app start.");

    deleteGroup("Added");
    QTest::qWait(1000);
    QVERIFY2(foundGroups.size() == 0, "Failed to delete Added group.");

    addSystem("localhost", 1);

    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after its deleting  and adding system.");

    deleteGroup("Added");
    QVERIFY2(foundGroups.size() == 0, "Failed to delete Added group.");

    addGroup("devil");
    addSystem("localhost", 1);

    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after its deleting  and adding system.");

}

//ready??
void PCTreeWidgetTestSuite::testContextMenu() {
    QSKIP("Ready.", SkipSingle);
    QMenu* menu = kernel->widget<QMenu*>("pctree_context_menu");
    QVERIFY2(menu, "Failed to get pctree_context_menu");
    QVERIFY2(!menu->isVisible(),"Context menu shouldn't be visible by default");

    PCTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    kernel->getMainWindow()->getPcTreeWidget()->showContextMenu(QPoint(0,0));
    QTest::qWait(1000);
    QVERIFY2(menu->isVisible(),"Context menu not visile after right click on PCTreeWidget");


}

//not ready - add some data
void PCTreeWidgetTestSuite::testAdressVerification_data() {

    //not valid domain?
    QTest::addColumn<QString>("system");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid IPv4")     << "127.0.0.1" << true;
    QTest::newRow("localhost")      << "localhost" << true;
    QTest::newRow("valid IPv6")     << "::1" << true;
    QTest::newRow("not valid IPv4") << "192.168.0.256" << false;
    QTest::newRow("not valid IPv6") << ":1" << false;
    QTest::newRow("valid domain")   << "www.google.com" << true;
}

//ready
void PCTreeWidgetTestSuite::testAdressVerification() {

    QSKIP("Ready.", SkipAll);
    QFETCH(QString, system);
    QFETCH(bool, success);

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    std::string systemStr = system.toStdString();
    addSystem(systemStr, success);

    QColor c = getSystem(systemStr, tree)->backgroundColor(0);
    std::stringstream ss;

    if (success && c == QColor("red")) {
        removeSystem(systemStr);
        ss << "Color of valid system is " << c.name().toStdString();
        ss << " but it should be " << QColor("white").name().toStdString();
        QFAIL(ss.str().c_str());
    }

    if (!success && c != QColor("red")) {
        removeSystem(systemStr);
        ss << "After program restart: Color of not valid system is " << c.name().toStdString();
        ss << " but it should be " << QColor("red").name().toStdString();
        QFAIL(ss.str().c_str());
    }

    cleanup();
    init();

    tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    c = getSystem(systemStr, tree)->backgroundColor(0);

    if (success && c == QColor("red")) {
        removeSystem(systemStr);
        ss << "After program restart: Color of valid system is " << c.name().toStdString();
        ss << " but it should be " << QColor("white").name().toStdString();
        QFAIL(ss.str().c_str());
    }

    if (!success && c != QColor("red")) {
        removeSystem(systemStr);
        ss << "After program restart: Color of not valid system is " << c.name().toStdString();
        ss << " but it should be " << QColor("red").name().toStdString();
        QFAIL(ss.str().c_str());
    }
    removeSystem(systemStr);

}

void PCTreeWidgetTestSuite::testAddSystems_data() {
    QTest::addColumn<int>("authType");

    QTest::newRow("press esc key") << 1;
    QTest::newRow("press cancel button") << 2;
    QTest::newRow("press ok button") << 3;
    QTest::newRow("not valid username, no password") << 4;
    QTest::newRow("valid username, no password") << 5;
    QTest::newRow("no user, valid password") << 6;
    QTest::newRow("not valid username, not valid password") << 7;
    QTest::newRow("valid username, not valid password") << 8;
    QTest::newRow("valid username, valid password") << 9;
}

//not ready - check correct behaviour after verification
void PCTreeWidgetTestSuite::testAddSystems() {
    QSKIP("Ready", SkipSingle);
    QFETCH(int, authType);

    addSystem("localhost", authType);
    if (authType < 9 && authType > 2) {
        while (!h->cleanupThread())
            QTest::qWait(1000);
    }
    removeSystem("localhost");
}

void PCTreeWidgetTestSuite::testTwoSystemsSameName() {
    QSKIP("Ready?", SkipSingle);
    addSystem("twin",1);
    addSystem("twin",1);
    std::vector<std::string> items;

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QTreeWidgetItem* addedItem = tree->findItems("Added", Qt::MatchExactly).at(0);
    getAllItems(addedItem, items, (char*)"twin");

    QVERIFY2(items.size() == 1, "There should not be possible to add two systems of same name");
}

//ready
void PCTreeWidgetTestSuite::testAddGroups() {
    QSKIP("Not develped now.", SkipSingle);

    std::vector<QTreeWidgetItem*> foundGroups;

    for (int i = 0; i < 2; i++) {
        addGroup("New group");

        QTest::qWait(1000);
        foundGroups = findGroup("New group");
        if (foundGroups.size() != 1) {
            QVERIFY2(foundGroups.size() != 0, "No top level item found.");
            if (i)
                QVERIFY2(foundGroups.size() == 1, "It is possible to add two groups of same name");
        }
    }
    deleteGroup("New group");
    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after test passing.");
}

//not ready
void PCTreeWidgetTestSuite::testRemoveGroup() {
    QSKIP("Not develped now.", SkipSingle);

    std::vector<QTreeWidgetItem*> foundGroups;

    addGroup("I am to stay");
    addGroup("Delete me");
    deleteGroup("Delete me");

    foundGroups = findGroup("I am to stay");
    QVERIFY2(foundGroups.size() == 1, "Deleting group removes more groups that it should.");

    deleteGroup("I am to stay");
    foundGroups = findGroup("Added");
    QVERIFY2(foundGroups.size() == 1, "Group Added not found after test passing.");
}


//ready??
void PCTreeWidgetTestSuite::testAddNamelessGroup() {
    QSKIP("Not needed.", SkipSingle);
    QAction* addGroups = kernel->widget<QAction*>("create_group_action");
    QVERIFY2(addGroups, "Failed to get create_group_action");

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    boost::thread t(boost::bind(&QAction::trigger,addGroups));
    QTest::qWait(1000);

    std::vector<QTreeWidgetItem*> foundGroups = findGroup("");
    if (foundGroups.size() != 1) {
        QFAIL("Can not find recently added group.");
    }
    QTest::keyClick(tree,Qt::Key_Enter);
    QTest::qWait(1000);
    t.join();
    foundGroups = findGroup("");
    QVERIFY2(foundGroups.size() == 0, "It is possible to add group without name set.");
}

void PCTreeWidgetTestSuite::testAddSystemsToGroups() {
    QSKIP("Not developed yet", SkipSingle);
}
