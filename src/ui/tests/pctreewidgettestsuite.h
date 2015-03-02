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

#ifndef PCTREEWIDGETTESTSUITE_H
#define PCTREEWIDGETTESTSUITE_H

#include "abstracttestsuite.h"

class PCTreeWidgetTestSuite : public AbstractTestSuite
{
    Q_OBJECT
private:
    void addGroup(std::string g);
    void deleteGroup(std::string g);
public:
    PCTreeWidgetTestSuite();
private slots:
    void testContextMenu();
    void testAddedGroupPresence();
    void testAdressVerification_data();
    void testAdressVerification();
    void testAddSystems_data();
    void testAddSystems();
    void testTwoSystemsSameName();
    void testAddGroups();
    void testRemoveGroup();
    void testAddNamelessGroup();
    void testAddSystemsToGroups();
};

#endif // PCTREEWIDGETTESTSUITE_H
