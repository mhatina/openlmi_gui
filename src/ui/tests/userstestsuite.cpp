#include "userstestsuite.h"

UsersTestSuite::UsersTestSuite() :
    AccountTestSuite()
{

}

UsersTestSuite::~UsersTestSuite()
{

}

void UsersTestSuite::manageUserAdding(int opt, QString name) {

    QDialog* newUserDialog = getWindow<QDialog*>("NewUserDialog");
    QPushButton *ok_button = newUserDialog->findChild<QPushButton*>("ok_button");
    QVERIFY2(newUserDialog, "Failed to get NewUserDialog");

    switch (opt) {
    case 1:
        QTest::qWait(500);
//        QTest::mouseClick(ok_button,Qt::LeftButton);
        ok_button->click();
        QTest::qWait(500);
        h->closeWarning();
        break;
    case 2:
        QLineEdit* name_line = newUserDialog->findChild<QLineEdit*>("name_line");
        QVERIFY2(name_line, "Failed to get name_line in newuserdialog");
        QTest::qWait(1000);
        QTest::keyClicks(name_line, name);
        QTest::qWait(1000);
//        QTest::mouseClick(ok_button,Qt::LeftButton);
        ok_button->click();
        break;
    }

}

void UsersTestSuite::closeUserAddingWindow() {
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
std::vector<std::string> UsersTestSuite::parsePasswd()
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
    sort(users.begin(), users.end(), AbstractPluginTestSuite::compareByFirstWord);
    return users;
}

void UsersTestSuite::addUser(int opt, QString name) {
    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QAction* addUserAction = kernel->widget<QAction*>("create_user_group_action");
    QVERIFY2(addUserAction, "Failed to get reate_user_group_action");

    boost::thread t(boost::bind(&UsersTestSuite::manageUserAdding, this, opt, name));
    addUserAction->trigger();
    while(!t.timed_join(boost::posix_time::millisec(100))) {
        QTest::qWait(1000);
    }
}

void UsersTestSuite::testUsersColumnNames()
{
    QSKIP("Ready", SkipAll);
    QFETCH(int, column_index);
    QFETCH(QString, column_name);

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QString name = userTable->horizontalHeaderItem(column_index)->text();
    QVERIFY(name == column_name);
}

void UsersTestSuite::testUsersColumnNames_data()
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

void UsersTestSuite::testUsers()
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

void UsersTestSuite::testAddUserDialog() {
    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(0);
    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QAction* addUserAction = kernel->widget<QAction*>("create_user_group_action");
    QVERIFY2(addUserAction, "Failed to get reate_user_group_action");

    boost::thread t(boost::bind(&UsersTestSuite::closeUserAddingWindow,this));
    addUserAction->trigger();
    while(!t.timed_join(boost::posix_time::millisec(100))) {
        QTest::qWait(1000);
    }
}

void UsersTestSuite::testAddUser() {

    QSKIP("Ready", SkipAll);
    QFETCH(int, opt);
    QFETCH(QString, name);
    prepareAccountPlugin(0);
    addUser(opt, name);
}

//TODO: finish after update include tests of new user in users table
void UsersTestSuite::testAddUser_data() {
    QTest::addColumn<int>("opt");
    QTest::addColumn<QString>("name");

    QTest::newRow("No entry data") << 1 << "";
    QTest::newRow("Just name") << 2 << "testuser1";
//    QTest::newRow("Fill all") << ;
//    QTest::newRow("") << ;
}

void UsersTestSuite::applyAdded() {
//    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(0);
    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    addUser(2, "lmicc_test_user");
    QTest::qWait(1000);
    QTest::mouseClick(apply_button, Qt::LeftButton);

    testUsers();
}
