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
TARGET          =       $$qtLibraryTarget(lmiccaccount)
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

SOURCES         +=      account.cpp \
                        instructions/accountinstruction.cpp \
                        instructions/groupinstruction.cpp \
                        ../../ui/instructions/instruction.cpp \
                        ../../ui/instructions/connectinstruction.cpp \
                        ../../ui/plugin.cpp \
                        dialogs/newuserdialog.cpp \
                        dialogs/newgroupdialog.cpp \
                        memberbox.cpp \
                        dialogs/groupmemberdialog.cpp \
                        instructions/getinstruction.cpp \
                        instructions/pushinstruction.cpp \
                        instructions/newgroupinstruction.cpp \
                        instructions/deletegroupinstruction.cpp \
                        instructions/addusertogroupinstruction.cpp \
                        instructions/removeuserfromgroupinstruction.cpp \
                        instructions/changegrouppropertyinstruction.cpp \
                        instructions/newuserinstruction.cpp \
                        instructions/deleteuserinstruction.cpp \
                        instructions/changeuserpropertyinstruction.cpp \
                        ../../ui/cimvalue.cpp \
                        ../../ui/cimdatetimeconv.cpp \
                        ../../ui/detailsdialog.cpp \
                        ../../ui/widgets/labeledlineedit.cpp

HEADERS         +=      account.h \
                        instructions/accountinstruction.h \
                        instructions/groupinstruction.h \
                        dialogs/newuserdialog.h \
                        dialogs/newgroupdialog.h \
                        memberbox.h \
                        ../../ui/instructions/connectinstruction.h \
                        dialogs/groupmemberdialog.h \
                        instructions/getinstruction.h \
                        instructions/pushinstruction.h \
                        instructions/newgroupinstruction.h \
                        instructions/deletegroupinstruction.h \
                        instructions/addusertogroupinstruction.h \
                        instructions/removeuserfromgroupinstruction.h \
                        instructions/changegrouppropertyinstruction.h \
                        instructions/newuserinstruction.h \
                        instructions/deleteuserinstruction.h \
                        instructions/changeuserpropertyinstruction.h \
                        ../../ui/plugin.h \
                        ../../ui/instructions/instruction.h \
                        ../../ui/detailsdialog.h \
                        ../../ui/widgets/labeledlineedit.h

FORMS           +=      account.ui \
                        dialogs/newuserdialog.ui \
                        dialogs/newgroupdialog.ui \
                        memberbox.ui \
                        dialogs/groupmemberdialog.ui

RESOURCES       =       icons.qrc
