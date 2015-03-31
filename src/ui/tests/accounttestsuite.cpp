#include "accounttestsuite.h"

#include <QTableWidget>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <iostream>

AccountTestSuite::AccountTestSuite() :
    AbstractPluginTestSuite(ACCOUNT)
{
}

void AccountTestSuite::prepareAccountPlugin(int tab) {
    Engine::IPlugin *plug = findPlugin("Account");
    QVERIFY2(plug, "Failed to find plugin \"Account\"");
    reselectLocalost();
    setTab("Account");

    QPushButton *refresh_button = kernel->widget<QPushButton *>("refresh_button");
    QVERIFY2(refresh_button, "Failed to get add_button");
    QTest::qWait(100);
    QTest::mouseClick(refresh_button, Qt::LeftButton);

    waitForRefresh(plug);

    if (tab) {
        QTabWidget* tabWid = plug->findChild<QTabWidget*>("tab_widget");
        QVERIFY2(tabWid, "Failed to get tab_widget");
        tabWid->setCurrentIndex(1);
        QTest::qWait(1000);
    }
    QTest::qWait(1000);

    std::stringstream ss;
    int i = 0;
    do {
        //source of segfault
        ss.str(plug->getRefreshInfo());
        if (ss.str().find(" 0 ") == std::string::npos) {
            break;
        }
        i++;
    } while (i < 5);
    QVERIFY2(i < 5, "No groups found");

    QTest::qWait(3000); // magic !!!!
}

//void AccountTestSuite::testContextMenu() {
//    Engine::IPlugin *plug = findPlugin("Account");
//    QVERIFY2(plug, "Failed to find plugin \"Account\"");

//    plug->customContextMenuRequested();

//}
