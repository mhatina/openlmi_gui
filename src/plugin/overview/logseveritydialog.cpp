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

#include "logseveritydialog.h"
#include "ui_logseveritydialog.h"

LogSeverityDialog::LogSeverityDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::LogSeverityDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Popup);
}

LogSeverityDialog::~LogSeverityDialog()
{
    delete m_ui;
}

int LogSeverityDialog::getSeverity()
{
    return m_ui->log_severity->currentIndex();
}

std::string LogSeverityDialog::getMessage()
{
    return m_ui->message->text().toStdString();
}

void LogSeverityDialog::setMessage(std::string message)
{
    m_ui->message->setText(message.c_str());
}
