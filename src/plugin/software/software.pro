# ***** BEGIN LICENSE BLOCK *****
#
#   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
#
#   This library is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as
#   published by the Free Software Foundation, either version 2.1 of the
#   License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#   MA 02110-1301 USA
#
# ***** END LICENSE BLOCK ***** */

QT              +=      core gui
TEMPLATE        =       lib
CONFIG          +=      plugin
QMAKE_CXXFLAGS  +=      -ansi -pedantic -Wall -Wextra
INCLUDEPATH     +=      ../../ui ../../ui/uics ../../logger

TARGET          =       $$qtLibraryTarget(lmiccsoftware)
DESTDIR         =       ../libs
DEFINES         +=      PEGASUS_PLATFORM_LINUX_X86_64_GNU

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
    target.path =       ../libs
    LIBS        +=      -L../../logger -llmicclogger
} else {
    target.path =       $$PREFIX$$LIB_PATH/lmicc
    manual_install {
        LIBS    +=      -L/usr/lib/lmicc -llmicclogger
    } else {
        LIBS    +=      -llmicclogger
    }
}
INSTALLS        +=      target

HEADERS         +=      software.h \
                        ../../ui/plugin.h \
                        ../../ui/instructions/connectinstruction.h \
                        ../../ui/instructions/instruction.h \
                        ../../ui/widgets/labeledlineedit.h \
                        ../../ui/detailsdialog.h \
                        listwidget.h \
                        instructions/enablerepoinstruction.h \
                        instructions/disablerepoinstruction.h \
                        instructions/verifypackageinstruction.h \
                        instructions/installpackageinstruction.h \
                        instructions/uninstallpackageinstruction.h \
                        instructions/updatepackageinstruction.h \
                        instructions/softwareinstruction.h \
                        installdialog.h

SOURCES         +=      software.cpp \
                        ../../ui/plugin.cpp \
                        ../../ui/instructions/connectinstruction.cpp \
                        ../../ui/instructions/instruction.cpp \
                        ../../ui/cimvalue.cpp \
                        ../../ui/widgets/labeledlineedit.cpp \
                        ../../ui/cimdatetimeconv.cpp \
                        ../../ui/detailsdialog.cpp \
                        listwidget.cpp \
                        instructions/enablerepoinstruction.cpp \
                        instructions/disablerepoinstruction.cpp \
                        instructions/verifypackageinstruction.cpp \
                        instructions/installpackageinstruction.cpp \
                        instructions/uninstallpackageinstruction.cpp \
                        instructions/updatepackageinstruction.cpp \
                        instructions/softwareinstruction.cpp \
                        installdialog.cpp

FORMS           +=      software.ui \
                        listwidget.ui \
                        installdialog.ui

RESOURCES       +=      ../../../icons/icons.qrc
