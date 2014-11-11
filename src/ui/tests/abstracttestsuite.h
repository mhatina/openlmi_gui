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


#ifndef ABSTRACTTESTSUITE_H
#define ABSTRACTTESTSUITE_H

#include <QObject>
#include <QtTest/QtTestGui>
#include <QtTest/QtTest>
#include "kernel.h"
#include "tst_helper.h"

class AbstractTestSuite: public QObject
{
    Q_OBJECT
protected:
    Engine::Kernel *kernel;
    Tst_Helper* h;

    template <typename T>
    T getWindow(std::string str) {
        T dialog = NULL;
        int i = 0;
        do {
            dialog = kernel->widget<T>(str);
            sleep(1);
            i++;
        } while (!dialog && i < 10);

        return dialog;
    }

    template <typename T>
    QList<T> getWindow() {
        QList<T> l;
        int i = 0;
        do {
            l = kernel->getMainWindow()->findChildren<T>();
            sleep(1);
            i++;
        } while (l.empty() && i < 10);

        return l;
    }

    void getAllItems(QTreeWidgetItem *item, std::vector<std::string> &items, char *name = NULL);
    void performAuthentication(int opt);
    void addSystem(std::string systemName, int opt = 0);
    void removeSystem(std::string name = "");

    QTreeWidgetItem* getSystem(std::string name, QTreeWidget *tree);
    std::vector<QTreeWidgetItem*>findGroup(std::string name);

public:
    explicit AbstractTestSuite(QObject *parent = 0);
    ~AbstractTestSuite();

protected slots:
    void init();
    void testRun();
    void cleanup();

signals:
    void haveDialog(QWidget *dialog, int opt);
};

#endif // ABSTRACTTESTSUITE_H
