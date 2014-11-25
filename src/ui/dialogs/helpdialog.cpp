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

#include "helpdialog.h"
#include "logger.h"

#include <QDir>
#include <QLibraryInfo>
#include <QProcess>

HelpDialog::HelpDialog() :
    m_process(NULL)
{
}

HelpDialog::~HelpDialog()
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished();
    }
    delete m_process;
}

void HelpDialog::showHelp(const QString &page)
{
    if (!start()) {
        return;
    }

    QByteArray ba("SetSource ");
    ba.append("qthelp://com.trolltech.examples.simpletextviewer/doc/");

    m_process->write(ba + page.toLocal8Bit() + '\0');
}

bool HelpDialog::start()
{
    if (!m_process) {
        m_process = new QProcess();
    }

    if (m_process->state() != QProcess::Running) {
        QString app = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator();
        app += "assistant";

        QStringList args;
        args << QLatin1String("-collectionFile")
             << QLibraryInfo::location(QLibraryInfo::ExamplesPath)
             + QLatin1String("/help/simpletextviewer/documentation/simpletextviewer.qhc")
             << QLatin1String("-enableRemoteControl");

        m_process->start(app, args);

        if (!m_process->waitForStarted()) {
            Logger::getInstance()->error("Unable to show help");
            return false;
        }
    }
    return true;
}
