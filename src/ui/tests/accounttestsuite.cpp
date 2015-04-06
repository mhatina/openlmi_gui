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
    preparePlugin("Account", tab);
}

//void AccountTestSuite::testContextMenu() {
//    Engine::IPlugin *plug = findPlugin("Account");
//    QVERIFY2(plug, "Failed to find plugin \"Account\"");

//    plug->customContextMenuRequested();

//}
