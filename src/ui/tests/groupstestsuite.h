#ifndef GROUPSTESTSUITE_H
#define GROUPSTESTSUITE_H

#include "accounttestsuite.h"

class GroupsTestSuite: public AccountTestSuite
{
    Q_OBJECT
public:
    GroupsTestSuite();
    std::vector<std::string> parseGroups();

private slots:
    void testGroups();
    void testAddGroup();
    void testDeleteGroup();
    void testModifyGroup();

private:
    void groupPresenceCheck();
    void addGroup(QString name);
    void deleteGroup(QString name);
    void manageGroupAdding(QString name);
    static bool compareByFirstNr(const std::string a, const std::string b) {
            return  atoi(a.c_str()) < atoi (b.c_str());
    }
};

#endif // GROUPSTESTSUITE_H
