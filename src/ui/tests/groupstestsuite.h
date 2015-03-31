#ifndef GROUPSTESTSUITE_H
#define GROUPSTESTSUITE_H

#include "accounttestsuite.h"

class GroupsTestSuite: public AccountTestSuite
{
    Q_OBJECT
public:
    GroupsTestSuite();
    void parseGroups();

private slots:
    void testGroups();
};

#endif // GROUPSTESTSUITE_H
