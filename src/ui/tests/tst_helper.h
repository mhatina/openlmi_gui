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

#ifndef TST_HELPER_H
#define TST_HELPER_H

#include <QDialog>
#include <QObject>
#include <QLineEdit>
#include <QPushButton>
#include <QtTestGui>
#include <QWidget>

#include "abstracttestsuite.h"

class AbstractTestSuite;

class Tst_Helper : public QObject
{
    Q_OBJECT


private:
    bool m_closed;
    bool m_success;
    boost::thread *t;
    AbstractTestSuite* ts;

public:
    void closeWarning();
    Tst_Helper(AbstractTestSuite *parent);

    bool getClosed() const;
    bool getSuccess();
    bool cleanupThread();

public slots:
    void closed();
    void evaluateAuthentification(QWidget *dialog, int opt);
};

#endif // TST_HELPER_H
