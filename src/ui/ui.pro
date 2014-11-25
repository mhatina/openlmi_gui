# ***** BEGIN LICENSE BLOCK *****
#   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2.1 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>.
# ***** END LICENSE BLOCK *****

QT              +=      core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET          =       lmicc
TEMPLATE        =       app
QMAKE_CXXFLAGS  +=      -ansi -pedantic -Wall -Wextra
LIBS            +=      -lslp -lpegclient -lpegcommon -lboost_thread -lgnome-keyring -lqscintilla2
INCLUDEPATH     +=      ../logger
DEFINES         +=      PEGASUS_PLATFORM_LINUX_X86_64_GNU
CONFIG          +=      link_pkgconfig qtestlib qscintilla2
PKGCONFIG       +=      glib-2.0

UI_DIR          =       uics
MOC_DIR         =       mocs
OBJECTS_DIR     =       objs

isEmpty(QT_SINGLE_APP) {
    QT_SINGLE_APP =     /usr/lib64/qt4/mkspecs/features/qtsingleapplication.prf
}
isEmpty(PREFIX) {
    PREFIX      =       /usr
}
isEmpty(LIB_PATH) {
    LIB_PATH    =       /lib
}

include($$QT_SINGLE_APP)

CONFIG(debug, debug|release) {
    DEFINES     +=      "PLUGIN_PATH=../plugin/libs"
    LIBS        +=      -L../logger -llmicclogger
} else {    
    target.path =       $$PREFIX/bin
    manual_install {
        DEFINES +=      "PLUGIN_PATH=$$PREFIX$$LIB_PATH/lmicc"
        LIBS    +=      -L$$PREFIX$$LIB_PATH -llmicclogger
    } else {
        DEFINES +=      "PLUGIN_PATH=/usr/$$LIB_PATH/lmicc"
        LIBS    +=      -llmicclogger        
    }
}
INSTALLS        +=      target

SOURCES         +=      main.cpp \
                        widgets/pctreewidget.cpp \
                        widgets/providerwidget.cpp \
                        discoverworker.cpp \
                        kernel.cpp \
                        cimdatetimeconv.cpp \
                        instructions/instruction.cpp \
                        instructions/connectinstruction.cpp \
                        plugin.cpp \
                        widgets/labeledlineedit.cpp \
                        kernelslots.cpp \
                        widgets/labeledlabel.cpp \
                        settings/generalsettings.cpp \
                        settings/isettings.cpp \
                        settings/generalpluginsettings.cpp \
                        widgets/treewidget.cpp \
                        widgets/progressbar.cpp \
                        cimclient.cpp \
                        cimvalue.cpp \
                        dialogs/helpdialog.cpp \
                        dialogs/addtreeitemdialog.cpp \
                        dialogs/authenticationdialog.cpp \
                        dialogs/codedialog.cpp \
                        dialogs/detailsdialog.cpp \
                        dialogs/logdialog.cpp \
                        dialogs/mainwindow.cpp \
                        dialogs/settingsdialog.cpp \
                        dialogs/systemdetailsdialog.cpp \
                        cimaddress.cpp \
                        widgets/treewidgetitem.cpp

HEADERS         +=      kernel.h \
                        plugin.h \
                        widgets/pctreewidget.h \
                        widgets/providerwidget.h \
                        discoverworker.h \
                        cimdatetimeconv.h \
                        instructions/instruction.h \
                        instructions/connectinstruction.h \
                        widgets/labeledlineedit.h \
                        widgets/labeledlabel.h \
                        settings/generalsettings.h \
                        settings/isettings.h \
                        settings/generalpluginsettings.h \
                        widgets/treewidget.h \
                        widgets/progressbar.h \
                        cimclient.h \
                        cimvalue.h \
                        dialogs/helpdialog.h \
                        cimaddress.h \
                        dialogs/addtreeitemdialog.h \
                        dialogs/authenticationdialog.h \
                        dialogs/codedialog.h \
                        dialogs/detailsdialog.h \
                        dialogs/logdialog.h \
                        dialogs/mainwindow.h \
                        dialogs/settingsdialog.h \
                        dialogs/systemdetailsdialog.h \
                        widgets/treewidgetitem.h

FORMS           +=      widgets/providerwidget.ui \
                        widgets/pctreewidget.ui \
                        widgets/labeledlineedit.ui \
                        widgets/labeledlabel.ui \
                        settings/generalsettings.ui \
                        settings/generalpluginsettings.ui \
                        dialogs/addtreeitemdialog.ui \
                        dialogs/authenticationdialog.ui \
                        dialogs/codedialog.ui \
                        dialogs/detailsdialog.ui \
                        dialogs/logdialog.ui \
                        dialogs/mainwindow.ui \
                        dialogs/settingsdialog.ui \
                        dialogs/systemdetailsdialog.ui

RESOURCES       +=      ../../icons/icons.qrc
