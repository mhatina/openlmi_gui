/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "generalsettings.h"
#include "logger.h"
#include "ui_generalsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent) :
    ISettings(parent),
    m_ui(new Ui::GeneralSettings)
{
    m_ui->setupUi(this);

    connect(m_ui->use_certificate_checkbox, SIGNAL(stateChanged(int)), this,
            SLOT(changeCertificate(int)));
}

GeneralSettings::~GeneralSettings()
{
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
}

std::string GeneralSettings::title()
{
    return "General";
}

void GeneralSettings::init()
{
    m_ui->terminal_emulator->setText("/usr/bin/xterm");
    m_ui->use_certificate_checkbox->setChecked(false);
    m_ui->certificate->setEnabled(false);
    m_ui->certificate->setText("/etc/pki/tls/certs");
    m_ui->log_archive->setText("");
}

void GeneralSettings::load(QFile &file)
{
    bool found = false;
    QXmlStreamReader in(&file);
    if (in.hasError()) {
        Logger::getInstance()->error(in.errorString().toStdString(), false);
        return;
    }

    std::string t = title();
    t.erase(remove_if(t.begin(), t.end(), isspace), t.end());

    while (!in.atEnd()) {
        QXmlStreamReader::TokenType token = in.readNext();

        if (in.name().toString().toStdString().empty() ||
            in.isEndElement()) {
            continue;
        }

        if (token == QXmlStreamReader::StartDocument) {
            continue;
        } else if (in.name().toString().toStdString() == t) {
            found = true;
        } else if (in.name() == "terminalEmulator") {
            while (!in.isCharacters()) {
                in.readNext();
            }
            QString emulator = in.text().toString();
            m_ui->terminal_emulator->setText(emulator);
        } else if (in.name() == "certificate") {
            QXmlStreamAttributes attr = in.attributes();
            bool enabled = (attr.value("enabled").toString().toStdString() == "true");
            m_ui->use_certificate_checkbox->setChecked(enabled);
            m_ui->certificate->setEnabled(enabled);

            while (!in.isCharacters()) {
                in.readNext();
            }
            std::string cert = in.text().toString().toStdString();
            m_ui->certificate->setText(cert.c_str());
        } else if (in.name() == "logArchive") {
            while (!in.isCharacters()) {
                in.readNext();
            }
            QString archive = in.text().toString();
            m_ui->log_archive->setText(archive);
        }
    }

    if (in.hasError()) {
        Logger::getInstance()->error(in.errorString().toStdString(), false);
        return;
    }

    if (!found)
        init();
}

void GeneralSettings::save(QXmlStreamWriter &writer)
{
    writer.writeTextElement("terminalEmulator", m_ui->terminal_emulator->text());

    writer.writeStartElement("certificate");
    writer.writeAttribute("enabled",
                          m_ui->use_certificate_checkbox->checkState() == Qt::Checked ? "true" : "false");
    writer.writeCharacters(m_ui->certificate->text());    
    writer.writeEndElement();

    writer.writeTextElement("logArchive", m_ui->log_archive->text());
}

void GeneralSettings::changeCertificate(int state)
{
    m_ui->certificate->setEnabled(state == Qt::Checked);
}
