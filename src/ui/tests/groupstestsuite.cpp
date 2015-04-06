#include "groupstestsuite.h"

GroupsTestSuite::GroupsTestSuite():
    AccountTestSuite()
{
}

void GroupsTestSuite::groupPresenceCheck() {

    std::vector<std::string> localGroups = parseGroups();

    QTableWidget* groupTable = kernel->widget<QTableWidget*>("group_table");
    QVERIFY2(groupTable, "Failed to get group_table widget");

    groupTable->sortItems(0);
    for (int i = 0; i < groupTable->rowCount(); i++) {
        std::string temp;
        temp.append(groupTable->item(i,0)->text().toStdString());
        temp.append(" ");
        temp.append(groupTable->item(i,1)->text().toStdString());

        if (temp != localGroups.at(i)) {
            std::cerr << "Groups differ:" << '\n'
                      << "/etc/group - " << localGroups.at(i) << '\n'
                      << "lmi -        " << temp << '\n';
            QFAIL("");
        }
    }
}

void GroupsTestSuite::testGroups() {
    QSKIP("Ready", SkipAll);
    prepareAccountPlugin(1);
    groupPresenceCheck();
}

std::vector<std::string> GroupsTestSuite::parseGroups() {
    using namespace std;

    vector<string> localGroups;
    FILE* groupFile;
    groupFile = fopen("/etc/group","r");
    if (groupFile == NULL) {
        qDebug("Failed to open /etc/group for reading.");
        return vector<string>();
    }

    char buff[256];
    memset(buff, 0, 256);
    while (fgets(buff, 256, groupFile)) {
        string tmp;
        char* pos = buff;
        buff[strlen(buff)-1] = '\0';
        pos = strtok(pos,":");
        pos = strtok(NULL,":");
        pos = strtok(NULL,":");
        tmp.append(pos);
        tmp.append(" ");
        tmp.append(buff);

        localGroups.push_back(tmp);
        memset(buff, 0, 256);
    }
    sort(localGroups.begin(), localGroups.end(), compareByFirstNr);
    return localGroups;
}

void GroupsTestSuite::manageGroupAdding(QString name) {
    QDialog* newGroupDialog = getWindow<QDialog*>("NewGroupDialog");
    QVERIFY2(newGroupDialog, "Failed to get NewGroupDialog");

    QLineEdit* name_line = newGroupDialog->findChild<QLineEdit*>("name_line");
    QVERIFY2(name_line, "Failed to get name_line in newGroupDialog");
    QPushButton *ok_button = newGroupDialog->findChild<QPushButton*>("ok_button");
    QVERIFY2(ok_button, "Failed to get ok_button in newGroupDialog");

    QTest::qWait(500);
    name_line->setText(name);
    QTest::qWait(500);
    ok_button->click();
    QTest::qWait(500);
}

void GroupsTestSuite::addGroup(QString name) {
    QAction* addGroupAction = kernel->widget<QAction*>("create_user_group_action");
    QVERIFY2(addGroupAction, "Failed to get reate_user_group_action");

    boost::thread t(boost::bind(&GroupsTestSuite::manageGroupAdding, this, name));
    addGroupAction->trigger();
    while(!t.timed_join(boost::posix_time::millisec(100))) {
        QTest::qWait(1000);
    }
}

void GroupsTestSuite::testAddGroup() {

    prepareAccountPlugin(1);

    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    addGroup("hola");
    Engine::IPlugin *pl = findPlugin("Account");

    QTest::qWait(1000);
    apply_button->click();

    QTest::qWait(500);
    waitForRefresh(pl);

    QTest::qWait(5000);
    groupPresenceCheck();
}

void GroupsTestSuite::testDeleteGroup() {

    QString name = "hola";
    prepareAccountPlugin(1);

    QTableWidget* groupTable = kernel->widget<QTableWidget*>("group_table");
    QVERIFY2(groupTable, "Failed to get group_table widget");

    QList<QTableWidgetItem*> itemsList = groupTable->findItems(name, Qt::MatchExactly);
    if (itemsList.size() == 0) {
        std::cerr << "Failed to find recently added group " << name.toStdString() << '\n';
        QFAIL("");
    }
    QTableWidgetItem* item = itemsList.at(0);
    groupTable->selectRow(item->row());

    QAction* deleteAction = kernel->widget<QAction*>("delete_user_group_action");
    QVERIFY2(deleteAction,"Failed to find delete_user_group_action.");

    QPushButton* apply_button = kernel->getMainWindow()->getToolbar()->findChild<QPushButton*>("apply_button");
    QVERIFY2(apply_button, "Failed to get apply_button");

    deleteAction->trigger();

    Engine::IPlugin *pl = findPlugin("Account");

    QTest::qWait(3000);
    apply_button->click();

    QTest::qWait(500);
    waitForRefresh(pl);

    QTest::qWait(5000);
    groupPresenceCheck();
}

void GroupsTestSuite::testModifyGroup() {
    //right now CIM doesn't support any of modifications of the group
}
