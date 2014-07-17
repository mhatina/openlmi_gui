/********************************************************************************
** Form generated from reading UI file 'softwareprovider.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOFTWAREPROVIDER_H
#define UI_SOFTWAREPROVIDER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SoftwareProviderPlugin
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *filter_box;
    QGridLayout *gridLayout;

    void setupUi(QWidget *SoftwareProviderPlugin)
    {
        if (SoftwareProviderPlugin->objectName().isEmpty())
            SoftwareProviderPlugin->setObjectName(QString::fromUtf8("SoftwareProviderPlugin"));
        SoftwareProviderPlugin->resize(885, 584);
        gridLayout_2 = new QGridLayout(SoftwareProviderPlugin);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        filter_box = new QGroupBox(SoftwareProviderPlugin);
        filter_box->setObjectName(QString::fromUtf8("filter_box"));
        gridLayout = new QGridLayout(filter_box);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 4, 0, 0);

        gridLayout_2->addWidget(filter_box, 0, 0, 1, 1);


        retranslateUi(SoftwareProviderPlugin);

        QMetaObject::connectSlotsByName(SoftwareProviderPlugin);
    } // setupUi

    void retranslateUi(QWidget *SoftwareProviderPlugin)
    {
        SoftwareProviderPlugin->setWindowTitle(QApplication::translate("SoftwareProviderPlugin", "Form", 0, QApplication::UnicodeUTF8));
        filter_box->setTitle(QApplication::translate("SoftwareProviderPlugin", "Filter:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SoftwareProviderPlugin: public Ui_SoftwareProviderPlugin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOFTWAREPROVIDER_H
