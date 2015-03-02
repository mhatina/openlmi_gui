#include "hardwareplugintestsuite.h"

#include <QGroupBox>
#include "widgets/labeledlabel.h"

HardwarePluginTestSuite::HardwarePluginTestSuite():
    AbstractPluginTestSuite(HARDWARE)
{
}

QTreeWidgetItem* HardwarePluginTestSuite::groupPresencetest(std::string s) {

    Engine::IPlugin* plug = findPlugin("Hardware");
    if (!plug) {
        errmsg = "Failed to find plugin \"Hardware\"";
        return NULL;
    }

    QTreeWidget* tree = plug->findChild<QTreeWidget*>("tree");
    if (!tree) {
        errmsg = "Failed to find tree in plugin Hardware";
        return NULL;
    }

    for(int i = 0; i < tree->topLevelItemCount(); i++) {
        if (tree->topLevelItem(i)->text(0).toStdString() == s) {
            return tree->topLevelItem(i);
        }
    }
    errmsg = "Failed to find group Base Peripheral";
    return NULL;
}


void HardwarePluginTestSuite::testBasePeripheral() {
    Engine::IPlugin* plug = findPlugin("Hardware");
    QVERIFY2(plug, "Failed to find plugin \"Hardware\"");

    reselectLocalost();
    setTab("Hardware");
    QTest::qWait(2000);

    waitForRefresh(plug);
    QTest::qWait(2000);
    QTreeWidgetItem* group = groupPresencetest("Base Peripheral");
    if (!group)
        QFAIL(errmsg.c_str());
    group->setExpanded(true);
    QTest::qWait(2000);

    /*docasne*/
    group = groupPresencetest("Battery");
        if (!group)
            QFAIL(errmsg.c_str());
        group->setExpanded(true);
        QTest::qWait(2000);

    pciCheck();
}


/*
 * TODO:
 *
 * find a way to find info about comp
 *
 */

void HardwarePluginTestSuite::pciCheck() {
    /*
    QGroupBox* deviceBox= kernel->widget<QGroupBox*>("device_box");
    QList<LabeledLabel> deti = deviceBox->layout()->findChildren<LabeledLabel>();
    QVERIFY(!deti.isEmpty());
    (
    */
}
