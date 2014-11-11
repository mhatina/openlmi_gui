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

#include "startuptestsuite.h"

StartupTestSuite::StartupTestSuite() :
    AbstractTestSuite()
{
}

StartupTestSuite::~StartupTestSuite()
{

}

//ready?
void StartupTestSuite::testVersionNr() {
//    QSKIP("Ready.", SkipSingle);
    std::string path = qApp->arguments().at(0).toStdString();

    unsigned found = path.rfind("src");
    QVERIFY2(found <= path.length(), "Not able to parse path to executable: No src directory found.");
    path.erase(found, path.length());

    path.append("lmicc.spec");
    std::ifstream spec (path.c_str());

    QVERIFY2(spec.is_open(),"Failed to find lmicc.specfile");
    std::string line;
    while(getline(spec, line) && line.substr(0, 8) != "Version:");
    line.erase(0, 8);
    line.erase(0,line.find_first_not_of(" "));
    QVERIFY2(LMI_VERSION == line, "LMI_VERSION is not set as in lmicc.spec file");
}

//ready?
void StartupTestSuite::testStartStatus()
{
//    QSKIP("Not develped now.", SkipSingle);
    QTreeWidget* tree(kernel->getMainWindow()->getPcTreeWidget()->getTree());
    QVERIFY2(tree, "Failed to get PcTreeWidget");
    int columnCount = tree->columnCount();
    int topLvlItemCount = tree->topLevelItemCount()
            ;
    std::vector<std::string> itemsBefore;
    getAllItems(tree->invisibleRootItem(), itemsBefore);

    cleanup();
    init();
    tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get PCTreeWidget");

    QVERIFY2(columnCount == tree->columnCount(),
             "PCTreeWidget has different count of collumn after restart.");
    QVERIFY2(topLvlItemCount == tree->topLevelItemCount(),
             "PCTreeWidget has different count of top level items after restart.");

    std::vector<std::string> itemsAfter;
    getAllItems(tree->invisibleRootItem(), itemsAfter);

    bool is_equal = false;
    if ( itemsBefore.size() < itemsAfter.size() )
        is_equal = std::equal ( itemsBefore.begin(), itemsBefore.end(), itemsAfter.begin() );
      else
        is_equal = std::equal ( itemsAfter.begin(), itemsAfter.end(), itemsBefore.begin() );

    QVERIFY2(is_equal,"Items in PCTreeWidget are different after restart");
}
