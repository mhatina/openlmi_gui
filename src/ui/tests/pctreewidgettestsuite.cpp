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

#include "pctreewidgettestsuite.h"

#include <QMenu>
#include <QMenuBar>

PCTreeWidgetTestSuite::PCTreeWidgetTestSuite() :
    AbstractTestSuite()
{
}

PCTreeWidgetTestSuite::~PCTreeWidgetTestSuite() {

}

//ready??
void PCTreeWidgetTestSuite::testContextMenu() {
//    QSKIP("Ready.", SkipSingle);
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

    QTest::newRow("valid IPv4") << "127.0.0.1" << true;
    QTest::newRow("localhost") << "localhost" << true;
    QTest::newRow("valid IPv6") << "::1" << true;
    QTest::newRow("not valid IPv4") << "192.168.0.256" << false;
    QTest::newRow("not valid IPv6") << ":1" << false;
    QTest::newRow("valid domain") << "www.google.com" << true;
}

//not ready - check systems after restart -> waiting for devel. op.
//zmenit vypis pri testAddress verification pridat nazov farby
//dokonciverifikacie po restarte (vyhadzuje segfault neviem preco)
void PCTreeWidgetTestSuite::testAdressVerification() {

    QSKIP("Segfault- neviem co s nim.", SkipAll);
    QFETCH(QString, system);
    QFETCH(bool, success);

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    std::string systemStr = system.toStdString();
    addSystem(systemStr, success);

    //check
    //pre docasne ucely
    QColor c = getSystem(systemStr, tree)->backgroundColor(0);

    std::stringstream ss;
    ss << "Color of valid system is " << c.name().toStdString() << " but it shouldn't have color set.";
    ss << "Color of added system is not white.";
    QVERIFY2(!success || c == QColor("white"), ss.str().c_str());


    //ss << "Color of added system is " << c << " but it should be red.";
    ss << "Color of not valid system should be red.";
    QVERIFY2(success || c == QColor("red"), ss.str().c_str());

    cleanup();
    init();

    tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    c = getSystem(systemStr, tree)->backgroundColor(0);

    ss << "After program restart: Color of added system is not white.";
    QVERIFY2(!success || c == QColor("white"), ss.str().c_str());

    ss << "After program restart: Color of not valid system should be red.";
    QVERIFY2(success || c == QColor("red"), ss.str().c_str());
    removeSystem(systemStr);

}

//not ready - check correct behaviour after verification
void PCTreeWidgetTestSuite::testAddSystems() {
    QSKIP("Segfault - neviem co s nim.", SkipSingle);
    for (int i = 3; i < 7; i++) {
        addSystem("localhost", i);

        //inetrnal purpose
//        QTest::qWait(1000);

        removeSystem("localhost");
    }

    addSystem("twin",1);
    addSystem("twin",1);
    std::vector<std::string> items;
    getAllItems(kernel->getMainWindow()->getPcTreeWidget()->getTree()->invisibleRootItem(), items, (char*)"twin");
    QVERIFY2(items.size() == 1, "There should not be possible to add two systems of same name");
}

//not ready - check group added is present
void PCTreeWidgetTestSuite::testAddGroups() {
//    QSKIP("Not develped now.", SkipSingle);
//    PCTreeWidget *pcTree = kernel->getMainWindow()->getPcTreeWidget();
//    QTest::mouseClick(pcTree,Qt::RightButton);
    QAction* addGroups = kernel->widget<QAction*>("create_group_action");

    QVERIFY2(addGroups, "Failed to get create_group_action");

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    for (int i = 0; i < 2; i++) {
        boost::thread t(boost::bind(&QAction::trigger,addGroups));
        QTest::qWait(1000);

        std::vector<QTreeWidgetItem*> foundGroups = findGroup("");
        QVERIFY2(foundGroups.size() == 1, "Can not find recently added group.");

        foundGroups.at(0)->setText(0, "New group");

        QTest::qWait(1000);
        foundGroups = findGroup("New group");
        if (foundGroups.size() != 1) {
            QVERIFY2(foundGroups.size() != 0, "No top level item found.");
            if (i)
                QVERIFY2(foundGroups.size() == 1, "It is possible to add two groups of same name");
        }
        t.join();
    }
}

//not ready
void PCTreeWidgetTestSuite::testRemoveGroup() {
//    QSKIP("Not develped now.", SkipSingle);
    QAction* deleteGroup = kernel->widget<QAction*>("delete_group_action");
    QVERIFY2(deleteGroup, "Failed to get delete_group_action");

    QAction* addGroups = kernel->widget<QAction*>("create_group_action");
    QVERIFY2(addGroups, "Failed to get create_group_action");

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    boost::thread t(boost::bind(&QAction::trigger,addGroups));
    QTest::qWait(1000);

    std::vector<QTreeWidgetItem*> foundGroups = findGroup("");
    QVERIFY2(foundGroups.size() == 1, "Can not find recently added group.");

    foundGroups.at(0)->setText(0, "Delete me");

    QTest::qWait(1000);
    foundGroups = findGroup("Delete me");
    if (foundGroups.size() != 1) {
        QVERIFY2(foundGroups.size() != 0, "Recently added group not found.");
    }
    t.join();

//    QPainter painter(tree);
//    painter.fillRect(tree->geometry(), QBrush(Qt::red, Qt::CrossPattern));
//    painter.fillRect(tree->visualItemRect(foundGroups.at(0)), QBrush(Qt::red, Qt::CrossPattern));
//    QTest::qWait(4000);

    //nejde mi to oznacit
    QTest::mouseClick(tree, Qt::LeftButton, 0);
    QPoint point = tree->mapToGlobal(tree->visualItemRect(foundGroups.at(0)).center());
    QCursor::setPos(point);
    QTest::qWait(1000);
    kernel->getMainWindow()->getPcTreeWidget()->showContextMenu(tree->mapFromGlobal(point));
    QTest::qWait(1000);
    QVERIFY2(deleteGroup->isEnabled(), "delete_group_action not enabled after rightclicking on group.");
    deleteGroup->trigger();

    QTest::qWait(1000);
    foundGroups = findGroup("Delete me");
    QVERIFY2(foundGroups.size() == 0, "Failed to remove group.");
}

//ready??
void PCTreeWidgetTestSuite::testAddNamelessGroup() {
//    QSKIP("Not needed.", SkipSingle);
    QAction* addGroups = kernel->widget<QAction*>("create_group_action");
    QVERIFY2(addGroups, "Failed to get create_group_action");

    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    boost::thread t(boost::bind(&QAction::trigger,addGroups));
    QTest::qWait(1000);

    std::vector<QTreeWidgetItem*> foundGroups = findGroup("");
    QVERIFY2(foundGroups.size() == 1, "Can not find recently added group.");

    QTest::keyClick(tree,Qt::Key_Enter);
    QTest::qWait(1000);
    foundGroups = findGroup("");
    QVERIFY2(foundGroups.size() == 0, "It is possible to add group without name set.");
    t.join();
}

void PCTreeWidgetTestSuite::testAddSystemsToGroups() {

}
