/* ***** BEGIN LICENSE BLOCK *****
 *
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 2.1 of the
 *   License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301 USA
 *
 * ***** END LICENSE BLOCK ***** */

#include <sstream>
#include <fstream>
#include <QDateTime>
#include <QMutex>
#include <Qt/qmessagebox.h>

#include <iostream>

#include "logger.h"

Logger *Logger::m_instance = NULL;
QMutex Logger::m_log_mutex;

Logger::Logger() :
    m_show_debug_message(DEBUGGING)
{
    setLogPath(DEFAULT_LOG_PATH);
    connect(
        this,
        SIGNAL(showDialog(String, String)),
        this,
        SLOT(displayDialog(String, String)));
}

Logger::~Logger()
{
}

Logger *Logger::getInstance()
{
    m_log_mutex.lock();
    if (m_instance == NULL) {
        m_instance = new Logger();
    }
    m_log_mutex.unlock();
    return m_instance;
}

void Logger::removeInstance()
{
    m_log_mutex.lock();
    if (m_instance != NULL) {
        delete m_instance;
        m_instance = NULL;
    }
    m_log_mutex.unlock();
}

const String Logger::getLogPath() const
{
    return m_log_path;
}

void Logger::setLogPath(String file_path)
{
    m_log_path = file_path;
}

bool Logger::info(String message, bool show_message)
{
    return log(
               message,
               INFO,
               show_message
           );
}

bool Logger::debug(String message, bool show_message)
{
    if (m_show_debug_message) {
        return log(
                   message,
                   DEBUG,
                   show_message
               );
    } else {
        return false;
    }
}

bool Logger::error(String message, bool show_message)
{
    if (show_message) {
        log(message,
            ERROR,
            false
           );

        emit showDialog("Warning", message);

        return true;
    }

    return log(message,
               ERROR,
               false
              );
}

bool Logger::critical(String message, bool show_message)
{
    if (message.find("connection timed out") != String::npos) {
        return error(message, show_message);
    }

    message += " Report this issue to developer.";
    if (show_message) {
        log(message,
            CRITICAL,
            false
           );

        emit showDialog("Critical", message);

        return true;
    }

    return log(message,
               CRITICAL,
               false
              );
}

void Logger::setShowDebug(bool value)
{
    m_show_debug_message = value;
}

// private

bool Logger::log(String message, log_mode mode, bool show_message)
{
    m_log_mutex.lock();
    m_log_file.open(
        m_log_path,
        std::ios_base::out | std::ios_base::app
    );
    if (!m_log_file.is_open()) {
        setLogPath(DEFAULT_LOG_PATH);
        m_log_mutex.unlock();
        String err =
            "Log file '" + m_log_path + "' cannot be opened!";
        if (message != err) {
            error(err);
        }
        return false;
    }

    if (show_message) {
        emit showMessage(message);
        emit showMessage(message.asQString());
    }

    m_log_file << "[" << getTime() << "] [" << getLogModeInfo(mode) << "] "
               << message << (message.at(message.size() - 1) == '\n' ? "" : "\n");
    m_log_file.close();
    m_log_mutex.unlock();

    return true;
}

String Logger::getTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%d %B %G %H:%M:%S", timeinfo);

    return buffer;
}

String Logger::getLogModeInfo(log_mode mode)
{
    String info[] = {
        "Info",
        "Debug",
        "Error",
        "Critical"
    };
    return info[mode];
}

void Logger::displayDialog(String title, String text)
{
    QMessageBox box;
    if (title == "Warning") {
        box.setObjectName("error_dialog");
        box.warning((QWidget *) parent(), title, text);
    } else {
        box.setObjectName("critical_dialog");
        box.critical((QWidget *) parent(), title, text);
    }
}
