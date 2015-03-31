#ifndef SERVICEPLUGINTESTSUITE_H
#define SERVICEPLUGINTESTSUITE_H


#include "abstractplugintestsuite.h"
#include "qtablewidget.h"

class ServicePluginTestSuite: public AbstractPluginTestSuite
{
    Q_OBJECT

private:
    void prepareServicePlugin();
    std::vector<std::string>* getLocalServices();
    void statusTranslation(std::vector<std::string>* services, char status[100]);

public:
    ServicePluginTestSuite();

private slots:
    void testColumnNames();
    void testColumnNames_data();
    void testServiceList();
//    void testActionBox();
    void testReload();
};

#endif // SERVICEPLUGINTESTSUITE_H
