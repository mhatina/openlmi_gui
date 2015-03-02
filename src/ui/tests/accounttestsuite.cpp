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


void AccountTestSuite::testUsersColumnNames_data()
{
    using namespace QTest;
    addColumn<int>("column_index");
    addColumn<QString>("column_name");

    newRow("Name")              << 0 << "Name";
    newRow("Full Name")         << 1 << "Full Name";
    newRow("UID")               << 2 << "UID";
    newRow("Home directory")    << 3 << "Home directory";
    newRow("Login shell")       << 4 << "Login shell";
}

void AccountTestSuite::testUsersColumnNames()
{
    QSKIP("Ready", SkipAll);
    QFETCH(int, column_index);
    QFETCH(QString, column_name);

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QString name = userTable->horizontalHeaderItem(column_index)->text();
    QVERIFY(name == column_name);
}

void AccountTestSuite::testSelectAccountPlugin()
{
    QSKIP("Not developed now", SkipAll);
    selectPlugin("Account");
}

void AccountTestSuite::manageUserAdding(int opt) {

    QDialog* newUserDialog = getWindow<QDialog*>("NewUserDialog");
    QPushButton *ok_button = newUserDialog->findChild<QPushButton*>("ok_button");
    QVERIFY2(newUserDialog, "Failed to get NewUserDialog");

    switch (opt) {
    case 1:
        QTest::qWait(500);
//        QTest::mouseClick(ok_button,Qt::LeftButton);
        ok_button->click();
        break;
    case 2:
        QLineEdit* name_line = newUserDialog->findChild<QLineEdit*>("name_line");
        QVERIFY2(name_line, "Failed to get name_line in newuserdialog");
        QTest::keyClicks(name_line, "testUser");
        QTest::qWait(1000);
        ok_button->click();
//        QTest::mouseClick(ok_button,Qt::LeftButton);
        break;
    }

}

//TODO: finish after update include tests of new user in users table
void AccountTestSuite::testAddUser_data() {
    QTest::addColumn<int>("opt");

    QTest::newRow("No entry data") << 1;
    QTest::newRow("Just name") << 2;
//    QTest::newRow("Fill all") << ;
//    QTest::newRow("") << ;
}

void AccountTestSuite::testAddUser() {

    QFETCH(int, opt);
    prepareAccountPlugin(0);
    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QAction* addUserAction = kernel->widget<QAction*>("create_user_group_action");
    QVERIFY2(addUserAction, "Failed to get reate_user_group_action");

    boost::thread t(boost::bind(&AccountTestSuite::manageUserAdding, this, opt));
    addUserAction->trigger();
    while(!t.timed_join(boost::posix_time::millisec(100))) {
        QTest::qWait(1000);
    }

}

void AccountTestSuite::testAddUserDialog() {
    prepareAccountPlugin(0);
    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QAction* addUserAction = kernel->widget<QAction*>("create_user_group_action");
    QVERIFY2(addUserAction, "Failed to get reate_user_group_action");

    boost::thread t(boost::bind(&AccountTestSuite::closeUserAddingWindow,this));
    addUserAction->trigger();
    while(!t.timed_join(boost::posix_time::millisec(100))) {
        QTest::qWait(1000);
    }
}

void AccountTestSuite::closeUserAddingWindow() {
    QDialog* newUserDialog = getWindow<QDialog*>("NewUserDialog");
    QVERIFY2(newUserDialog, "Failed to get NewUserDialog");
    QTest::qWait(1000);
    QPushButton *button = newUserDialog->findChild<QPushButton *>("cancel_button");
    /*
     * TODO: change to QTest::mouseClick()
     */
    button->click();
    QTest::qWait(1000);
}

void AccountTestSuite::testGroups() {
    QSKIP("Not ready", SkipAll);
    prepareAccountPlugin(1);
}

void AccountTestSuite::testUsers()
{
    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(0);
    std::vector<std::string> locUsers = parsePasswd();
    std::vector<std::string> olmiUsers;

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    for(int j = 0; j < userTable->rowCount(); j++) {
        std::stringstream buff("");
        for (int i = 0; i < userTable->columnCount(); i++) {
            if (userTable->item(j, i) != NULL) {
                buff << userTable->item(j, i)->text().toStdString();
                if (i != 4)
                    buff << " ";
            }
        }
        //std::cerr << j << ": " << buff.str() << std::endl;
        QVERIFY(buff.str() == locUsers.at(j));
        olmiUsers.push_back(buff.str());

    }

    QVERIFY2(locUsers == olmiUsers, "Users are not same.");

}

bool compareUsers(const std::string a, const std::string b)
{
    if (a.find(" ") != std::string::npos && b.find(" ") != std::string::npos) {
        std::string lowerA(a.substr(0, a.find(" ")));
        std::string lowerB(b.substr(0, b.find(" ")));
        std::transform(lowerA.begin(),lowerA.end(),lowerA.begin(), ::tolower);
        std::transform(lowerB.begin(),lowerB.end(),lowerB.begin(), ::tolower);
        return  lowerA < lowerB;
    }
    return false;
}

std::vector<std::string> AccountTestSuite::parsePasswd()
{
    using namespace std;
    ifstream passwd;
    passwd.open("/etc/passwd");
    if (!passwd.is_open()) {
        qDebug("Failed to open /etc/passwd for reading.");
        return vector<string>();
    }
    vector<string> users;
    string instr;
    while (getline(passwd, instr)) {
        string uid;
        stringstream user;
        size_t pos = 0;
        int i = 0;

        while ((pos = instr.find(":")) != string::npos) {
            i++;
            if (i == 2 || i == 4) {
                instr.erase(0, pos + 1);
                continue;
            }
            if (i == 3) {
                uid = instr.substr(0, pos);
                instr.erase(0, pos + 1);
                continue;
            }
            user << instr.substr(0, pos) << " ";
            if (i == 5) {
                user << uid << " ";
            }
            instr.erase(0, pos + 1);
        }
        user << instr;
        users.push_back(user.str());
//        std::cerr << "_" << user.str() << "_"<< std::endl;
    }
    passwd.close();
    sort(users.begin(), users.end(), compareUsers);
    return users;
}

//void AccountTestSuite::testContextMenu() {
//    Engine::IPlugin *plug = findPlugin("Account");
//    QVERIFY2(plug, "Failed to find plugin \"Account\"");

//    plug->customContextMenuRequested();

//}
