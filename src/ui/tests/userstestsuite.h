#ifndef USERSTESTSUITE_H
#define USERSTESTSUITE_H

#include "accounttestsuite.h"
#include "qtablewidget.h"

class UsersTestSuite : public AccountTestSuite
{
    Q_OBJECT
public:
    UsersTestSuite();
    ~UsersTestSuite();

private:
    void manageUserAdding(int opt, QString name);
    void closeUserAddingWindow();
    std::vector<std::string> parsePasswd();
    void addUser(int opt, QString name);

private slots:
    //    void testUserDeletion();
    //    void testUserDeletion_data();
        void testUsersColumnNames();
        void testUsersColumnNames_data();
        void testUsers();

        void testAddUserDialog();
        void testAddUser();
        void testAddUser_data();
        void applyAdded();
//        void revertAdded();
};

#endif // USERSTESTSUITE_H
