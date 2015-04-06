#ifndef ABSTRACTPLUGINTESTSUITE_H
#define ABSTRACTPLUGINTESTSUITE_H

#include "abstracttestsuite.h"

typedef enum {OWERVIEW, ACCOUNT, HARDWARE, NETWORK, SERVICE, SOFTWARE} PLUGIN_TYPE;
class AbstractPluginTestSuite: public AbstractTestSuite
{
    Q_OBJECT
private:
    PLUGIN_TYPE plugType;
public:
    AbstractPluginTestSuite(PLUGIN_TYPE t);
    void reselectLocalost();
    void setTab(std::string name);
protected:
    void waitForRefresh(Engine::IPlugin* plug);
    void preparePlugin(std::string plugName, int opt = 0);
private slots:
    void initTestCase();
    void cleanupTestCase();
};

#endif // ABSTRACTPLUGINTESTSUITE_H
