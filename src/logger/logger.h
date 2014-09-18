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

#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Macro for showing debug logs
 *
 * Implicitly set to false (debug logs are hidden). If debug logs
 * shouldn't be hidden set to true when building project.
 *
 */
#ifndef DEBUGGING
#define DEBUGGING false
#endif

#include "logger_global.h"

#include <QObject>
#include <QMutex>
#include <string>
#include <fstream>

/**
 *  @brief Enum of log modes
 */
typedef enum {
    INFO,
    DEBUG,
    ERROR,
    CRITICAL
} log_mode;

/**
 * @brief The Logger class.
 *
 * Singleton class that provides easy way of logging.
 */
class LOGGERSHARED_EXPORT Logger : public QObject
{
    Q_OBJECT

signals:
    /**
     * @brief Signal for informing main gui that message should be displayed for user.
     * @param message -- text that should be displayed
     */
    void showMessage(std::string message);
    void showMessage(QString message);
    void showDialog(std::string title, std::string text);

private:
    bool m_show_debug_message;
    std::ofstream m_log_file;
    std::string m_log_path;
    static QMutex m_mutex;
    static Logger *m_instance;

    /**
     * @brief General method for logging
     *
     * @param message -- text that is logged
     * @param mode -- log mode
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool log(std::string message, log_mode mode, bool show_message);

    /**
     * @brief getTime
     * @return current time
     */
    std::string getTime();

    /**
     * @brief getLogModeInfo
     * @param mode -- log mode
     * @return text representation of mode
     */
    std::string getLogModeInfo(log_mode mode);

public:
    /**
     * @brief Constructor
     */
    Logger();

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Create instance if one does not exist
     * @return instace of Logger
     */
    static Logger *getInstance();

    /**
     * @brief Delete instance if one exists
     */
    static void removeInstance();

    /**
     * @brief Getter
     * @return path to log file
     */
    const std::string getLogPath() const;

    /**
     * @brief Setter
     * @param file_path -- path to new log file
     */
    void setLogPath(std::string file_path);

    /**
     * @brief Info log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool info(std::string message, bool show_message = true);

    /**
     * @brief Debug log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool debug(std::string message, bool show_message = true);

    /**
     * @brief Error log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool error(std::string message, bool show_message = true);

    /**
     * @brief Critical log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool critical(std::string message, bool show_message = true);

    void setShowDebug(bool value);

    private
slots:
    void displayDialog(std::string title, std::string text);
};

#endif // LOGGER_H
