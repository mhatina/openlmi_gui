#ifndef ACCOUNTTESTSUITE_H
#define ACCOUNTTESTSUITE_H

#include "abstractplugintestsuite.h"
#include "qtablewidget.h"

class AccountTestSuite: public AbstractPluginTestSuite
{
    Q_OBJECT
public:
    AccountTestSuite();

protected:
    void prepareAccountPlugin(int tab);
};

#endif // ACCOUNTTESTSUITE_H
