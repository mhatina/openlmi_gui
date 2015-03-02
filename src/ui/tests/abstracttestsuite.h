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


#ifndef ABSTRACTTESTSUITE_H
#define ABSTRACTTESTSUITE_H

#include <QObject>
#include <QtTest/QtTestGui>
#include <QtTest/QtTest>
#include "plugin.h"
#include "kernel.h"
#include "tst_helper.h"

class Tst_Helper;


class AbstractTestSuite: public QObject
{
    Q_OBJECT
    friend class Tst_Helper;
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
        } while (!dialog && i < 5);

        return dialog;
    }

    template <typename T>
    QList<T> getWindow() {
        QList<T> l;
        int i = 0;
        do {
            l = kernel->getMainWindow()->findChildren<T>();
            usleep(100000);
            i++;
        } while (l.empty() && i < 300);

        return l;
    }

    void getAllItems(QTreeWidgetItem *item, std::vector<std::string> &items, char *name = NULL);
    void performAuthentication(int opt);
    void addSystem(std::string systemName, int opt = 0);
    void removeSystem(std::string name = "");
    void selectPlugin(std::string s);

    Engine::IPlugin* findPlugin(std::string s);
    QTreeWidgetItem* getSystem(std::string name, QTreeWidget *tree);
    std::vector<QTreeWidgetItem*>findGroup(std::string name);

public:
    void status();
    explicit AbstractTestSuite(QObject *parent = 0);
    ~AbstractTestSuite();

protected slots:
    void init();
    void cleanup();

signals:
    void haveDialog(QWidget *dialog, int opt);
};

#endif // ABSTRACTTESTSUITE_H
