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
LIBS            +=      -lslp -lpegclient -lpegcommon -lboost_thread -lgnome-keyring
INCLUDEPATH     +=      ../logger ../plugin
DEFINES         +=      PEGASUS_PLATFORM_LINUX_X86_64_GNU
CONFIG          +=      link_pkgconfig
PKGCONFIG       +=      glib-2.0
RESOURCES       +=      icons.qrc
CONFIG          +=      qtestlib

UI_DIR          =       uics
MOC_DIR         =       mocs
OBJECTS_DIR     =       objs

isEmpty(PREFIX) {
    PREFIX      =       /usr
}
isEmpty(LIB_PATH) {
    LIB_PATH    =       /lib
}

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
                        mainwindow.cpp \
                        widgets/pctreewidget.cpp \
                        widgets/providerwidget.cpp \
                        discoverworker.cpp \
                        authenticationdialog.cpp \
                        kernel.cpp \
                        cimdatetimeconv.cpp \
                        instructions/instruction.cpp \
                        instructions/connectinstruction.cpp \
                        plugin.cpp \
                        showtextdialog.cpp \
                        widgets/labeledlineedit.cpp \
                        detailsdialog.cpp \
                        kernelslots.cpp \
                        treewidgetitem.cpp \
                        systemdetailsdialog.cpp \
                        settingsdialog.cpp \
                        widgets/labeledlabel.cpp \
                        settings/generalsettings.cpp \
                        settings/isettings.cpp \
                        settings/generalpluginsettings.cpp \
                        widgets/treewidget.cpp \
                        widgets/progressbar.cpp \
                        address.cpp \
                        cimclient.cpp \
                        cimvalue.cpp \
                        logdialog.cpp \
                        tests/abstracttestsuite.cpp \
                        tests/menubartestsuite.cpp \
                        tests/tst_lmicc.cpp \
                        tests/toolbartestsuite.cpp \
                        tests/pctreewidgettestsuite.cpp \
                        tests/startuptestsuite.cpp \
                        tests/tst_helper.cpp \
    tests/accounttestsuite.cpp \
    tests/hardwareplugintestsuite.cpp \
    tests/abstractplugintestsuite.cpp \
    tests/handler.cpp \
    tests/userstestsuite.cpp \
    tests/serviceplugintestsuite.cpp \
    tests/groupstestsuite.cpp

HEADERS         +=      mainwindow.h \
                        kernel.h \
                        plugin.h \
                        widgets/pctreewidget.h \
                        widgets/providerwidget.h \
                        discoverworker.h \
                        authenticationdialog.h \
                        cimdatetimeconv.h \
                        instructions/instruction.h \
                        instructions/connectinstruction.h \
                        showtextdialog.h \
                        widgets/labeledlineedit.h \
                        detailsdialog.h \
                        treewidgetitem.h \
                        systemdetailsdialog.h \
                        settingsdialog.h \
                        widgets/labeledlabel.h \
                        settings/generalsettings.h \
                        settings/isettings.h \
                        settings/generalpluginsettings.h \
                        widgets/treewidget.h \
                        widgets/progressbar.h \
                        address.h \
                        cimclient.h \
                        cimvalue.h \
                        logdialog.h \
                        tests/abstracttestsuite.h \
                        tests/menubartestsuite.h \
                        tests/tst_lmicc.h \
                        tests/toolbartestsuite.h \
                        tests/pctreewidgettestsuite.h \
                        tests/startuptestsuite.h \
                        tests/tst_helper.h \
    tests/accounttestsuite.h \
    tests/hardwareplugintestsuite.h \
    tests/abstractplugintestsuite.h \
    tests/handler.h \
    tests/userstestsuite.h \
    tests/serviceplugintestsuite.h \
    tests/groupstestsuite.h

FORMS           +=      mainwindow.ui \
                        widgets/providerwidget.ui \
                        widgets/pctreewidget.ui \
                        authenticationdialog.ui \
                        showtextdialog.ui \
                        widgets/labeledlineedit.ui \
                        detailsdialog.ui \
                        systemdetailsdialog.ui \
                        settingsdialog.ui \
                        widgets/labeledlabel.ui \
                        settings/generalsettings.ui \
                        settings/generalpluginsettings.ui \
                        logdialog.ui
