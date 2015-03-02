#ifndef ACCOUNTTESTSUITE_H
#define ACCOUNTTESTSUITE_H

#include "abstractplugintestsuite.h"
class AccountTestSuite: public AbstractPluginTestSuite
{
    Q_OBJECT
public:
    AccountTestSuite();

//private:
//    struct compareUsers {
//        bool operator() (const std::string a, const std::string b) const {
//            return a.substr(0,a.find(" ")).compare(b.substr(0,b.find(" ")));
//        }
//    };


private:
    std::vector<std::string> parsePasswd();
    std::vector<std::string> parseGroup();
    void prepareAccountPlugin(int tab);
    void manageUserAdding(int opt);
    void closeUserAddingWindow();
private slots:
//    void testContextMenu();
    void testAddUserDialog();
    void testAddUser();
    void testAddUser_data();
    void testSelectAccountPlugin();
    void testUsers();
    void testUsersColumnNames();
    void testUsersColumnNames_data();
    void testGroups();
};

#endif // ACCOUNTTESTSUITE_H
