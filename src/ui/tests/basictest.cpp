#include "basictest.h"
#include "kernel.h"
#include "helps.h"

#include <QTreeWidgetItem>
#include <QtTest/QtTestGui>
#include <QtTest/QTest>


BasicTest::BasicTest(QObject *parent) :
    QObject(parent)
{

}

int BasicTest::main_for_tests () {
    BasicTest *bt = new BasicTest();

    return QTest::qExec(bt);
}

void BasicTest::init() {
    kernel = new Engine::Kernel();
    kernel->showMainWindow();
}

void  BasicTest::cleanup() {
    delete kernel;
}

void BasicTest::testCloseApp() {
    QAction *exit = kernel->widget<QAction *>("action_exit");
    QVERIFY2(exit, "Failed to get action_exit Widget");
    QVERIFY2(exit->isVisible(), "Exit menu item is not visible");
    Helps *h = new Helps();

    QObject::connect(
        kernel->getMainWindow(),
        SIGNAL(closing()),
        h,
        SLOT(closed()));

    QVERIFY2(!h->getClosed(), "Main window is not visible");
    exit->trigger();
    QVERIFY2(h->getClosed(), "Main window is visible");
    delete h;

}

void BasicTest::testReloadPluginsAction() {
    QTabWidget *plugins = kernel->widget<QTabWidget*>("tab_widget");
    QVERIFY2(plugins, "Failed to get tab_widget");
    QAction *reloadPluginsAct = kernel->widget<QAction*>("action_reload_plugins");
    QVERIFY2(reloadPluginsAct, "Failed to get action_reload_plugins widget");

    QVERIFY2(plugins->count() > 0, "No plugins loaded at start");

    kernel->deletePlugins();
    QVERIFY2(plugins->count() == 0, "Plugins found after delete");

    reloadPluginsAct->trigger();
    QVERIFY2(plugins->count() > 0, "No plugins loaded after reload");
}

void BasicTest::testAddButton(){
    PCTreeWidget* pcTree = kernel->getMainWindow()->getPcTreeWidget();
    QPushButton *addButton = kernel->widget<QPushButton*>("main_window_add_button");
    QVERIFY2(addButton, "Failed to get add_button");
    QTest::qWait(100);

    QTest::mouseClick(addButton,Qt::LeftButton, 0, QPoint(), 1000);

    QList<QTreeWidgetItem*> items = pcTree->getTree()->selectedItems();
    if(items.size() != 1) {
        QVERIFY2(items.size(), "No item added after clicking add button");
        QVERIFY2(items.size() > 1, "More items added after clicking add button");
    }

    //todo selectedItems check size == 1
    items[0]->setText(0, "test");

    // TODO press esc key for

    QDialog* dialog = kernel->widget<QDialog*>("AuthenticationDialog");
    QVERIFY2(dialog, "Failed to get AuthenticationDialog widget");
    QTest::keyClick(dialog, Qt::Key_Escape, 0, 50);
}

void BasicTest::testOpenLMIShell() {
    QPushButton* button = kernel->widget<QPushButton*>("show_code_button");
    QVERIFY2(button, "Failed to get show_code_button");

    QTest::qWait(100);
    QTest::mouseClick(button, Qt::LeftButton);
    QTest::qWait(100);
    //TODO dokonci po taboch
}

void BasicTest::testStartStatus() {


//    QTreeWidget treeBeforeRestart(*(kernel->getMainWindow()->getPcTreeWidget()->getTree()));
//    QVERIFY2(treeBeforeRestart, "Failed to get QtreeWidget");
//    cleanup();

//    init();
//    QTreeWidget treeAfterRestart = kernel->getMainWindow()->getPcTreeWidget()->getTree();
//    QVERIFY2(treeBeforeRestart, "Failed to get QtreeWidget");

//    QVERIFY2(treeAfterRestart==treeBeforeRestart, "QTreeWidget changed after application restart."); //porovnavaj vlastnosti postupne

}

void BasicTest::testShowInfo() {
    QAction *infoButton = kernel->widget<QAction *>("action_about");
    QVERIFY2(infoButton != NULL, "Failed to get action_about widget");

//    {
//        QDialog *detail = kernel->widget<QDialog *>("");
//        QVERIFY2(detail == NULL, "Not null before action_about triggering");
//    }

//    boost::thread(boost::bind(&QAction::trigger,infoButton));

//    qDebug(qApp->activeWindow()->objectName().toStdString().c_str());
//    QTest::qWait(100);
//    QDialog *detail = kernel->widget<QDialog *>("DetailsDialog");
//    QVERIFY2(detail != NULL, "DetailsDialog  null after action_about->trigger()");
//    QVERIFY2(detail->isVisible(), "DetailsDialog is not active after action_about->trigger()");

}

