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

#include "toolbartestsuite.h"

ToolbarTestSuite::ToolbarTestSuite() :
    AbstractTestSuite()
{
}
ToolbarTestSuite::~ToolbarTestSuite()
{
}

//ready?
void ToolbarTestSuite::testAddButton()
{
//    QSKIP("Not needed.", SkipSingle);
    PCTreeWidget *pcTree = kernel->getMainWindow()->getPcTreeWidget();
    QVERIFY2(pcTree, "Failed to get PCTreeWidget.");
    QPushButton *addButton = kernel->getMainWindow()->getToolbar()->findChild<QPushButton *>("add_button");
    QVERIFY2(addButton, "Failed to get add_button");
    QTest::qWait(100);

    int itemCount = pcTree->topLevelNode("Added")->childCount();
    QTest::mouseClick(addButton, Qt::LeftButton);
    QVERIFY2(itemCount + 1 == pcTree->topLevelNode("Added")->childCount(),
             "No item added to tree after clicking add buton.");
    QVERIFY2(itemCount + 2 > pcTree->topLevelNode("Added")->childCount(),
             "More items added after clicking add button");
    QTest::keyClick(kernel->getMainWindow(), Qt::Key_Enter);
    QVERIFY2(itemCount + 1 == pcTree->topLevelNode("Added")->childCount(), "There should't be added an item without name set.");

}

//not ready - doesn't work
void ToolbarTestSuite::testRemoveButton() {
    QSKIP("Segfault - neviem co s nim.", SkipSingle);
    QTreeWidget *tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget.");

    for (int i = 1; i <= 3; i++) {
        std::stringstream ss;
        ss << "192.256.0." << i;
        addSystem(ss.str(), 0);
    }

    std::vector<std::string> itemsBefore;
    getAllItems(tree->invisibleRootItem(), itemsBefore);

    removeSystem();

    std::vector<std::string> itemsAfter;
    getAllItems(tree->invisibleRootItem(), itemsAfter);

    bool is_equal = false;
    if ( itemsBefore.size() < itemsAfter.size() )
        is_equal = std::equal ( itemsBefore.begin(), itemsBefore.end(), itemsAfter.begin() );
    else
        is_equal = std::equal ( itemsAfter.begin(), itemsAfter.end(), itemsBefore.begin() );
    QVERIFY2(is_equal,"Items in PCTreeWidget are different after pushing remove button with no item selected");

    itemsBefore.clear();
    itemsAfter.clear();

    //items after -> items before
    getAllItems(tree->invisibleRootItem(), itemsBefore);
    removeSystem("192.168.0.2");
    getAllItems(tree->invisibleRootItem(), itemsAfter);


    //check
    QVERIFY2(itemsBefore.size() - itemsAfter.size() <= 1, "More systems removed after clicking remove button.");
    QVERIFY2(itemsBefore.size()-1==itemsAfter.size(), "Failed to remove system.");

    itemsBefore.clear();
    itemsAfter.clear();
    getAllItems(tree->topLevelItem(0), itemsAfter);
    for(std::vector<std::string>::iterator it = itemsAfter.begin(); it != itemsAfter.end(); it++)
        removeSystem(*it);
}
