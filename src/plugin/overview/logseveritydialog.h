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

#ifndef LOGSEVERITYDIALOG_H
#define LOGSEVERITYDIALOG_H

#include <QDialog>
#include "lmi_string.h"

const char *const log_severity[] = {
    "Unknown", "Other",
    "Information",
    "Degraded/Warning",
    "Minor", "Major",
    "Critical",
    "Fatal/NonRecoverable"
};

const char *const syslog_severity[] = {
    "Emergency", "Alert",
    "Critical", "Error",
    "Warning", "Notice",
    "Informational",
    "Debug"
};

namespace Ui
{
class LogSeverityDialog;
}

class LogSeverityDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::LogSeverityDialog *m_ui;

public:
    explicit LogSeverityDialog(QWidget *parent = 0);
    ~LogSeverityDialog();

    int getSeverity();
    String getMessage();
    void setMessage(String message);

};

#endif // LOGSEVERITYDIALOG_H
