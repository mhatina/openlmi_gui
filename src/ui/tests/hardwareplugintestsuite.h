#ifndef HARDWAREPLUGINTESTSUITE_H
#define HARDWAREPLUGINTESTSUITE_H

#include "abstractplugintestsuite.h"

class HardwarePluginTestSuite: public AbstractPluginTestSuite
{
    Q_OBJECT
public:
    HardwarePluginTestSuite();
    std::string errmsg;

private:
    QTreeWidgetItem* groupPresencetest(std::string s);
    void pciCheck();
private slots:
    void testBasePeripheral();
//    void testBattery();
//    void testBridge();
//    void testChassis();
//    void testDataAcqAndSig();
//    void testDisplayController();
//    void testMassStorageContr();
//    void testMemory();
//    void testMultimedia();
//    void testNetworkContr();
//    void testPhysicalPort();
//    void testProcessor();
//    void testSerialBus();
//    void testSimpleCommunication();
};
#endif // HARDWAREPLUGINTESTSUITE_H
