#include "userstestsuite.h"
#include "detailsdialog.h"
#include <QDialogButtonBox>

UsersTestSuite::UsersTestSuite() :
    AccountTestSuite()
{

}

UsersTestSuite::~UsersTestSuite()
{

}

void UsersTestSuite::manageUserAdding(int opt, QString name) {

    QDialog* newUserDialog = getWindow<QDialog*>("NewUserDialog");
    QVERIFY2(newUserDialog, "Failed to get NewUserDialog");
    QPushButton *ok_button = newUserDialog->findChild<QPushButton*>("ok_button");

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
        name_line->setText(name);
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

void UsersTestSuite::usersPresenceCheck() {

    std::vector<std::string> locUsers = parsePasswd();
    std::vector<std::string> olmiUsers;

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    userTable->sortItems(0);
    QTest::qWait(500);
    for(int j = 0; j < userTable->rowCount(); j++) {
        std::string buff("");
        for (int i = 0; i < userTable->columnCount(); i++) {
            if (userTable->item(j, i) != NULL) {
                buff.append(userTable->item(j, i)->text().toStdString());
                if (i != 4)
                    buff.append(" ");
            }
        }
        if (buff.c_str() != locUsers.at(j)) {
            std::cerr << "Users differ:\n"
                      << "/etc/passwd - " << locUsers.at(j) << '\n'
                      << "LMICC       - " << buff << '\n';
            QFAIL("");
        }
        olmiUsers.push_back(buff);

    }
}

void UsersTestSuite::testUsers()
{
    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(0);
    usersPresenceCheck();

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

void UsersTestSuite::testApplyAdded() {

    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(0);

    addUser(2, "lmicc_test_user");
    QTest::qWait(1000);

    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    Engine::IPlugin *pl = findPlugin("Account");

    QTest::qWait(1000);
    apply_button->click();

    QTest::qWait(500);
    waitForRefresh(pl);

    QTest::qWait(5000);
    usersPresenceCheck();
    deleteUser("lmicc_test_user");

}

void UsersTestSuite::deleteUser(std::string name) {

    prepareAccountPlugin(0);

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QList<QTableWidgetItem*> itemsList = userTable->findItems(name.c_str(), Qt::MatchExactly);
    if (itemsList.size() == 0) {
        std::cerr << "Failed to find recently added user " << name << '\n';
        QFAIL("");
    }
    QTableWidgetItem* item = itemsList.at(0);
    userTable->selectRow(item->row());

    QAction* deleteAction = kernel->widget<QAction*>("delete_user_group_action");
    QVERIFY2(deleteAction,"Failed to find delete_user_group_action.");

    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    deleteAction->trigger();

    Engine::IPlugin *pl = findPlugin("Account");

    QTest::qWait(1000);
    apply_button->click();

    QTest::qWait(500);
    waitForRefresh(pl);

    QTest::qWait(5000);
    usersPresenceCheck();
}

void UsersTestSuite::testModifyUser() {
    std::string name = "user_to_be_modified";
    prepareAccountPlugin(0);
    addUser(2, name.c_str());

    QTest::qWait(1000);

    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    Engine::IPlugin *pl = findPlugin("Account");

    QTest::qWait(1000);
    apply_button->click();

    QTest::qWait(500);
    waitForRefresh(pl);

    moveCursorAndSelect(name);

    QTest::qWait(500);
    boost::thread t(boost::bind(&UsersTestSuite::manageModifying, this));

    system("xdotool click --repeat 2 1");

    // TODO: active wait, new thread: change shell, apply

    QTest::qWait(2000);
    apply_button->click();

    waitForRefresh(pl);
    moveCursorAndSelect(name);
    system("xdotool click 1");

    checkModified();

    QTest::qWait(5000);
    deleteUser(name);
}

void UsersTestSuite::moveCursorAndSelect(std::string name) {

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");

    QList<QTableWidgetItem*> itemsList = userTable->findItems(name.c_str(), Qt::MatchExactly);
    if (itemsList.size() == 0) {
        std::cerr << "Failed to find recently added user " << name << '\n';
        QFAIL("");
    }
    QTableWidgetItem* item = itemsList.at(0);
    userTable->selectRow(item->row());
    QTest::qWait(1000);

    QRect item_rect = userTable->visualItemRect(item);
    QTest::mouseMove(userTable->viewport(), item_rect.center());

}

void UsersTestSuite::manageModifying() {
    QTest::qWait(1000);

    DetailsDialog* dialog = (DetailsDialog*) qApp->activeModalWidget();
    QLayout* layout = dialog->findChild<QWidget*>("widget_area")->layout();
    QVERIFY2(layout, "Failed to find layout in DetailsDialog");

    for (int i = 0; i < layout->count(); i++) {
        QWidget* item = layout->itemAt(i)->widget();
        if (item->objectName() == "LoginShell") {
            ((LabeledLineEdit*) item)->setText("/bin/sh");
            break;
        }
    }

    QTest::qWait(1000);
    QDialogButtonBox* buttonBox = dialog->findChild<QDialogButtonBox*>("button_box");
    buttonBox->button(QDialogButtonBox::Ok)->click();
}

void UsersTestSuite::checkModified() {
    QTest::qWait(4000);

    QTableWidget *userTable = kernel->widget<QTableWidget *>("user_table");
    QVERIFY2(userTable, "Failed to get user_table widget");
    QVERIFY2(userTable->selectedItems()[4]->text() == "/bin/sh", "Failed to modify login shell");
}
