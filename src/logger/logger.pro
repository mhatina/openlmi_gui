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

QT       -= gui

TARGET = logger
TEMPLATE = lib
INCLUDEPATH += ../ui

DEFINES += LOGGER_LIBRARY

UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs

SOURCES += logger.cpp

HEADERS += logger.h\
        logger_global.h

linux-g++:contains(QMAKE_HOST.arch, x86_64):{
    target.path = /usr/lib64
} else {
    target.path = /usr/lib
}
DEFINES +=DEFAULT_LOG_PATH="\\\"/tmp/openlmi_log\\\""
INSTALLS += target

RESOURCES +=
