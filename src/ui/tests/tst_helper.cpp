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

#include "tst_helper.h"

#include "abstracttestsuite.h"

bool Tst_Helper::getClosed() const
{
    return m_closed;
}


bool Tst_Helper::getSuccess()
{
    bool tmp = m_success;
    m_success = false;
    return tmp;
}

bool Tst_Helper::cleanupThread()
{
    if (t != NULL && t->joinable()) {
        if (!t->timed_join(boost::posix_time::millisec(100)))
                return false;
        delete t;
        t = NULL;
        return true;
    }

    return false;
}

Tst_Helper::Tst_Helper(AbstractTestSuite *parent) :
    m_closed(false),
    m_success(false)
{
    t = NULL;
    ts = parent;
}

void Tst_Helper::closed()
{
    m_closed = true;
}

void getAllItems(QTreeWidgetItem *item, std::vector<std::string> &items, char *name)
{
    for ( int i = 0; i < item->childCount(); ++i ) {
        if (!name || item->child(i)->text(0).compare(name)) {
            items.push_back(item->child(i)->text(0).toStdString());
        }
        getAllItems(item->child(i), items, NULL);
    }
}

void Tst_Helper::closeWarning()
{
    QList<QMessageBox *> messBoxes = ts->getWindow<QMessageBox *>();
    QVERIFY2(messBoxes.size() == 1, "None or more warning windows found");
    QVERIFY2(messBoxes.at(0), "Failed to get warning window");
    QVERIFY2(messBoxes.at(0)->isVisible(), "Warning window is not visible");    
    QTest::keyClick(messBoxes.at(0), Qt::Key_Escape);
    QTest::qWait(1000);
}

void Tst_Helper::evaluateAuthentification(QWidget *dialog, int opt)
{    
    QPushButton *okButton;
    QLineEdit *username_line;
    QLineEdit *passwd_line;
    //data preparation
    if (opt > 2) {
        okButton = dialog->findChild<QPushButton *>("ok_button");
        QVERIFY2(okButton, "Failed to get ok_button");
    }
    if (opt > 3 && opt != 6) {
        username_line = dialog->findChild<QLineEdit *>("username_line");
        QVERIFY2(username_line, "Failed to get username_line");
    }
    if (opt > 5) {
        passwd_line = dialog->findChild<QLineEdit *>("passwd_line");
        QVERIFY2(passwd_line, "Failed to get username_line");
    }

    switch (opt) {
    case 1: {
        //just esc
        QTest::keyClick(dialog, Qt::Key_Escape, 0, 50);
        QVERIFY2(dialog, "AuthenticationDialog not null after escape key pressing");
        break;
    }
    case 2: {
        //just cancel
        QPushButton *cnclbutton = dialog->findChild<QPushButton *>("cancel_button");
        QVERIFY2(cnclbutton, "Failed to get cancel_button");

        QTest::mouseClick(cnclbutton, Qt::LeftButton);
        QVERIFY2(dialog, "AuthenticationDialog not null after clicking cancel_button");
        break;
    }
    case 3: {
        //just ok
        break;
    }
    case 4: {
        //not correct username no passw
        QTest::keyClicks(username_line, "testUser");
        break;
    }
    case 5: {
        //correct username no passw
        QTest::keyClicks(username_line, "root");
        //TODO test of expected behaviour
        break;
    }
    case 6: {
        //passw and no user
        QTest::keyClicks(passwd_line, "password");
        //TODO test of expected behaviour
        break;
    }
    case 7: {
        //not correct passwd, not correct user
        QTest::keyClicks(username_line, "testUser");
        QTest::keyClicks(passwd_line, "password");
        break;
    }
    case 8: {
        //correct user not correct passw

        QTest::keyClicks(username_line, "root");
        QTest::keyClicks(passwd_line, "password");
        //TODO test of expected behaviour
        break;
    }
    case 9: {
        //correct passw and user
        QTest::keyClicks(username_line, "root");
        QTest::keyClicks(passwd_line, "redhat");
        //TODO test of expected behaviour
        break;
    }
    default:
        m_success = false;
        return;
    }
    if (opt > 2) {
        QTest::qWait(500);
        if (opt != 9) {
            t = new boost::thread(boost::bind(&Tst_Helper::closeWarning, this));
        }

        QTest::mouseClick(okButton, Qt::LeftButton);
        QVERIFY2(dialog, "AuthenticationDialog not null after clicking ok_button");
    }
    m_success = true;
}


