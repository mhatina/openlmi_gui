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

#include <QObject>
#include <QMutex>
#include <fstream>

#include "logger_global.h"
#include "lmi_string.h"


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
    void showMessage(String message);
    void showMessage(QString message);
    void showDialog(String title, String text);

private:
    bool m_show_debug_message;
    std::ofstream m_log_file;
    String m_log_path;
    static QMutex m_log_mutex;
    static Logger *m_instance;

    /**
     * @brief General method for logging
     *
     * @param message -- text that is logged
     * @param mode -- log mode
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool log(String message, log_mode mode, bool show_message);

    /**
     * @brief getLogModeInfo
     * @param mode -- log mode
     * @return text representation of mode
     */
    String getLogModeInfo(log_mode mode);

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
    const String getLogPath() const;

    /**
     * @brief Setter
     * @param file_path -- path to new log file
     */
    void setLogPath(String file_path);

    /**
     * @brief Info log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool info(String message, bool show_message = true);

    /**
     * @brief Debug log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool debug(String message, bool show_message = true);

    /**
     * @brief Error log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool error(String message, bool show_message = true);

    /**
     * @brief Critical log
     * @param message -- text that is logged
     * @param show_message -- whether @ref showMessage() should be emitted
     * @return true if successful; otherwise false
     */
    bool critical(String message, bool show_message = true);

    /**
     * @brief getTime
     * @return current time
     */
    String getTime();

    void setShowDebug(bool value);

    private
slots:
    void displayDialog(String title, String text);
};

#endif // LOGGER_H
