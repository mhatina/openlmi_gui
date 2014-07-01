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

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = openLMI
TEMPLATE = app
QMAKE_CXXFLAGS += -ansi -pedantic -Wall -Wextra
LIBS += -lslp -lpegclient -lpegcommon -lboost_thread -lgnome-keyring -llogger
INCLUDEPATH += ../logger
DEFINES += PEGASUS_PLATFORM_LINUX_X86_64_GNU
CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0
RESOURCES += \
    icons.qrc

UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs

SOURCES += main.cpp\
    mainwindow.cpp \
    widgets/pctreewidget.cpp \
    widgets/providerwidget.cpp \
    discoverworker.cpp \
    authenticationdialog.cpp \
    kernel.cpp \
    lmiwbem_addr.cpp \
    lmiwbem_client.cpp \
    lmiwbem_value.cpp \
    cimdatetimeconv.cpp \
    instructions/instruction.cpp \
    instructions/connectinstruction.cpp \
    plugin.cpp \
    showtextdialog.cpp \
    eventlog.cpp \
    widgets/labeledlineedit.cpp \
    detailsdialog.cpp

HEADERS  += mainwindow.h \
    kernel.h \
    plugin.h \
    widgets/pctreewidget.h \
    widgets/providerwidget.h \
    config.h \
    discoverworker.h \
    authenticationdialog.h \
    lmiwbem_addr.h \
    lmiwbem_client.h \
    lmiwbem_value.h \
    cimdatetimeconv.h \
    instructions/instruction.h \
    instructions/connectinstruction.h \
    showtextdialog.h \
    eventlog.h \
    widgets/labeledlineedit.h \
    detailsdialog.h

FORMS    += mainwindow.ui \
    widgets/providerwidget.ui \
    widgets/pctreewidget.ui \
    authenticationdialog.ui \
    showtextdialog.ui \
    widgets/labeledlineedit.ui \
    detailsdialog.ui

linux-g++:contains(QMAKE_HOST.arch, x86_64):{
    DEFINES+= "PLUGIN_PATH=\\\"/usr/lib64/openlmi\\\""
} else {
    DEFINES+= "PLUGIN_PATH=\\\"/usr/lib/openlmi\\\""
}

target.path = /usr/bin
INSTALLS += target

