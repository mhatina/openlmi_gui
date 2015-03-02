#include "abstractplugintestsuite.h"

AbstractPluginTestSuite::AbstractPluginTestSuite(PLUGIN_TYPE t):
    AbstractTestSuite()
{
    plugType = t;
}

void AbstractPluginTestSuite::initTestCase() {
    init();
    addSystem("localhost", 9);
    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get tree in PCTreeWidget");
    cleanup();
    //NOTE: potentially dangerous!!
    system("sed -i 's/true/false/g' ~/.config/lmicc_settings.xml ");
}

void AbstractPluginTestSuite::cleanupTestCase() {
    init();
    removeSystem("localhost");
    cleanup();
}

void AbstractPluginTestSuite::reselectLocalost() {
    QTreeWidget* tree = kernel->getMainWindow()->getPcTreeWidget()->getTree();
    QVERIFY2(tree, "Failed to get tree in PCTreeWidget");

    getSystem("localhost", tree)->setSelected(false);
    getSystem("localhost", tree)->setSelected(true);
}

void AbstractPluginTestSuite::setTab(std::string name) {
    QTabWidget* tw = kernel->widget<QTabWidget*>("tab_widget");

    for (int i = 0; i < tw->count(); i++) {
        if (tw->tabText(i) == name.c_str()) {
            tw->setCurrentIndex(i);
            return;
        }
    }
    std::string errmsg("Failed to get tab called ");
    errmsg.append(name);
    QFAIL(errmsg.c_str());
}

void AbstractPluginTestSuite::waitForRefresh(Engine::IPlugin* plug) {
    int i = 0;
    while(i < 10) {
        if(plug->isRefreshed())
            break;
        QTest::qWait(1000);
        i++;
    }
}
