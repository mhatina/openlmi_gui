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

QT              -=      gui

TARGET          =       lmicclogger
TEMPLATE        =       lib
INCLUDEPATH     +=      ../ui

DEFINES         +=      LOGGER_LIBRARY
DEFINES         +=      PEGASUS_PLATFORM_LINUX_X86_64_GNU

UI_DIR          =       uics
MOC_DIR         =       mocs
OBJECTS_DIR     =       objs

SOURCES         +=      logger.cpp \
    ../ui/lmi_string.cpp

HEADERS         +=      logger.h \
                        logger_global.h

isEmpty(PREFIX) {
    PREFIX = /usr
}
isEmpty(LIB_PATH) {
    LIB_PATH    =       /lib
}

CONFIG(debug, debug|release) {
    DEFINES     +=      "DEBUGGING=false"
} else {
    target.path =   $$PREFIX$$LIB_PATH
}

DEFINES         +=      DEFAULT_LOG_PATH="\\\"/tmp/lmicc_log\\\""
INSTALLS        +=      target
