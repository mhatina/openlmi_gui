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

void AbstractPluginTestSuite::preparePlugin(std::string plugName, int opt) {

    Engine::IPlugin *plug = findPlugin(plugName);
    if (plug == NULL) {
        std::cerr << "Failed to find plugin " << plugName << '\n';
        QFAIL("");
    }
    reselectLocalost();
    setTab(plugName);

    QPushButton *refresh_button = kernel->widget<QPushButton *>("refresh_button");
    QVERIFY2(refresh_button, "Failed to get refresh_button");
    QTest::qWait(100);

    while (plug->isRefreshing())
        QTest::qWait(1000);
    QTest::mouseClick(refresh_button, Qt::LeftButton);

    waitForRefresh(plug);

    QTest::qWait(1000);

    if (opt) {
        QTabWidget* tabWid = plug->findChild<QTabWidget*>("tab_widget");
        QVERIFY2(tabWid, "Failed to get tab_widget");
        tabWid->setCurrentIndex(1);
        waitForRefresh(plug);

    }
    QTest::qWait(1000);

    std::string str;
    int i = 0;
    do {
        str.append(plug->getRefreshInfo());
        if (str.find(" 0 ") == std::string::npos) {
            break;
        }
        i++;
    } while (i < 5);
    QVERIFY2(i < 5, "No groups found");

    QTest::qWait(3000); // magic !!!!
}

void AbstractPluginTestSuite::setTab(std::string name) {
    QTabWidget* tw = kernel->widget<QTabWidget*>("tab_widget");

    for (int i = 0; i < tw->count(); i++) {
        if (tw->tabText(i) == name.c_str()) {
            tw->setCurrentIndex(i);
            return;
        }
    }
    std::string errmsg("Failed to get tab ");
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
