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

#include "tst_helper.h"

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

Tst_Helper::Tst_Helper() :
    m_closed(false),
    m_success(false)
{
}

void Tst_Helper::closed()
{
    m_closed = true;
}


void Tst_Helper::evaluateAuthentification(QWidget *dialog, int opt)
{
    /*pre docasne ucely*/
    if (opt > 6) {
        QTest::keyClick(dialog, Qt::Key_Escape, 0, 50);
        QVERIFY2(dialog, "AuthenticationDialog not null after escape key pressing");
        m_success = true;
        return;
    }

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
        QPushButton *button = dialog->findChild<QPushButton *>("cancel_button");
        QVERIFY2(button, "Failed to get cancel_button");
        QTest::mouseClick(button, Qt::LeftButton);
        QVERIFY2(dialog, "AuthenticationDialog not null after clicking cancel_button");
        break;
    }
    case 3: {
        //just ok

//        QList<QMessageBox*> messBoxes = Tst_LMICC::getWindow<QMessageBox>();
//        QVERIFY2(messBoxes, "Failed to get warning window");
//        QVERIFY2(messBoxes.size() == 1, "None or more warning windows found");
//        QVERIFY2(messBoxes.at(0),"Failed to get warning window");
//        QVERIFY2(messBoxes.at(0)->isVisible(), "Warning window is not visible");
//        QTest::qWait(1000);
        //TODO test of expected behaviour
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
        QTest::qWait(1000);
        QTest::mouseClick(okButton, Qt::LeftButton);
    }
    m_success = true;
}
