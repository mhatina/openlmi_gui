#ifndef BASICTEST_H
#define BASICTEST_H

#include <QObject>
#include <QtTest/QtTestGui>
#include <QtTest/QtTest>
#include "kernel.h"

class BasicTest : public QObject
{
    Q_OBJECT
private:
    Engine::Kernel *kernel;

private slots:
    void init();
    void cleanup();
    void testCloseApp();
    void testReloadPluginsAction();
    void testShowInfo();

    void testStartStatus();

    void testAddButton();
    void testOpenLMIShell();
public:
    static int main_for_tests();
    explicit BasicTest(QObject *parent = 0);

signals:
};
#endif // BASICTEST_H
