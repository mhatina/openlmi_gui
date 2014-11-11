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

#include "abstracttestsuite.h"

AbstractTestSuite::AbstractTestSuite(QObject *parent) :
    QObject(parent),
    h(new Tst_Helper())
{
    connect(
        this,
        SIGNAL(haveDialog(QWidget *, int)),
        h,
        SLOT(evaluateAuthentification(QWidget *, int)));
}

AbstractTestSuite::~AbstractTestSuite()
{
    delete h;
}

void AbstractTestSuite::testRun() {
    QVERIFY(false);
}

void AbstractTestSuite::init()
{
    kernel = new Engine::Kernel();
    MainWindow *window = kernel->getMainWindow();
    if (!window) {
        qDebug("Failed to get main window");
    }
    connect(
        window,
        SIGNAL(closing()),
        h,
        SLOT(closed()));
    kernel->showMainWindow();
}

//nechyba tu nieco? pr - zatvorenie programu
void AbstractTestSuite::cleanup()
{
    delete kernel;
}

//nebol by lepsi ukazatel? - nie, lebo potrebujem si udrzat informacie o danych systemoch aj po restarte
void AbstractTestSuite::getAllItems(QTreeWidgetItem *item, std::vector<std::string> &items, char *name)
{
    for ( int i = 0; i < item->childCount(); ++i ) {
        if (!name || item->child(i)->text(0).compare(name)) {
            items.push_back(item->child(i)->text(0).toStdString());
        }
        getAllItems(item->child(i), items);
    }
}

void AbstractTestSuite::performAuthentication(int opt)
{
    QWidget *window;
    if (opt == 0) {
        QList<QMessageBox *> childrenList = getWindow<QMessageBox *>();
        QVERIFY2(!childrenList.isEmpty(), "Failed to get error_dialog");
        window = childrenList.at(0);
        opt = 1;
    } else {
        window = getWindow<QDialog *>("AuthenticationDialog");
        QVERIFY2(window, "Failed to get AuthenticationDialog");
    }
    emit haveDialog(window, opt);
    int i = 0;
    bool result = false;
    do {
        sleep(1);
        i++;
    } while (!(result = h->getSuccess()) && i < 10);

    QVERIFY2(result, "Failed to perform authentification");
}

void AbstractTestSuite::addSystem(std::string systemName, int opt)
{

    PCTreeWidget *pcTree = kernel->getMainWindow()->getPcTreeWidget();
    QVERIFY2(pcTree, "Failed to get PCTreeWidget.");
    QPushButton *addButton = kernel->getMainWindow()->getToolbar()->findChild<QPushButton *>("add_button");
    QVERIFY2(addButton, "Failed to get add_button");

    if (opt > 7) {
        QTest::qWait(1000);
        removeSystem(systemName);
        QTest::qWait(10000);
    }

    QTest::qWait(100);
    QTest::mouseClick(addButton, Qt::LeftButton);


    boost::thread t(boost::bind(&AbstractTestSuite::performAuthentication, this, opt));
    pcTree->getTree()->selectedItems().at(0)->setText(0, systemName.c_str());
    QTest::qWait(1000);
    t.join();
}

//not ready
void AbstractTestSuite::removeSystem(std::string name)
{

    QPushButton *removeButton = kernel->getMainWindow()->getToolbar()->findChild<QPushButton *>("remove_button");
    QVERIFY2(removeButton, "Failed to get remove_button");

    //zmenit!!
    if (name != "") {
        QTreeWidget *tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
        QVERIFY2(tree, "Failed to get QTree.");
        getSystem(name.c_str(), tree)->setSelected(true);
    }

    QTest::mouseClick(removeButton, Qt::LeftButton);
}

QTreeWidgetItem *AbstractTestSuite::getSystem(std::string name, QTreeWidget *tree)
{
    QList<QTreeWidgetItem *> items = tree->findItems(name.c_str(), Qt::MatchContains | Qt::MatchRecursive);

    std::cerr << name << std::endl;
    if (items.isEmpty()) {
        qDebug("Error: Failed to find item %s in QTree.", name.c_str());
        return NULL;
    }
    if (items.size() != 1) {
        qDebug("Error: More items called %s found.", name.c_str());
    }
    return items.at(0);
}

//najskor otestuj, ci mozu byt dve skupiny rovnakeho mena
std::vector<QTreeWidgetItem *> AbstractTestSuite::findGroup(std::string name)
{
    QTreeWidget *tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    if (!tree) {
        qDebug("failed to get TreeWidget");
    }

    std::vector<QTreeWidgetItem *> foundTopLvlItems;

    for (int i = 0; i < tree->topLevelItemCount(); i++) {
        QTreeWidgetItem *tmp = tree->topLevelItem(i);
        if (tmp && tmp->text(0).toStdString() == name) {
            foundTopLvlItems.push_back(tree->topLevelItem(i));
        }
    }
    return foundTopLvlItems;
}
