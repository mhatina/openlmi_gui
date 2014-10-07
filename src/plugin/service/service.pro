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
TARGET          =       $$qtLibraryTarget(lmiccservice)
DESTDIR         =       ../libs
LIBS            +=      -lpegclient -lpegcommon -lboost_thread
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

HEADERS         +=      service.h \
                        ../../ui/plugin.h \
                        instructions/startserviceinstruction.h \
                        instructions/stopserviceinstruction.h \
                        instructions/enableserviceinstruction.h \
                        instructions/getinstruction.h \
                        instructions/disableserviceinstruction.h \
                        instructions/serviceinstruction.h \
                        actionbox.h \
                        ../../ui/instructions/connectinstruction.h \
                        instructions/restartserviceinstruction.h \
                        instructions/reloadserviceinstruction.h \
                        ../../ui/instructions/instruction.h \
                        ../../ui/detailsdialog.h \
                        ../../ui/widgets/labeledlineedit.h

SOURCES         +=      service.cpp \
                        ../../ui/plugin.cpp \
                        ../../ui/cimdatetimeconv.cpp \
                        ../../ui/lmiwbem_value.cpp \
                        instructions/startserviceinstruction.cpp \
                        instructions/stopserviceinstruction.cpp \
                        instructions/enableserviceinstruction.cpp \
                        instructions/getinstruction.cpp \
                        instructions/disableserviceinstruction.cpp \
                        instructions/serviceinstruction.cpp \
                        actionbox.cpp \
                        ../../ui/instructions/connectinstruction.cpp \
                        ../../ui/instructions/instruction.cpp \
                        instructions/restartserviceinstruction.cpp \
                        instructions/reloadserviceinstruction.cpp \
                        ../../ui/detailsdialog.cpp \
                        ../../ui/widgets/labeledlineedit.cpp

FORMS           +=      service.ui \
                        actionbox.ui

RESOURCES       +=      icons.qrc