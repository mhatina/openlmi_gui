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

#include "logger.h"
#include "settings/generalsettings.h"
#include "settings/generalpluginsettings.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QPushButton>

#define SETTINGS_FILE_PATH "/home/mhatina/.openlmi/openlmi_settings.xml"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);

    connect(
        m_ui->list,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(change()));
    connect(
        m_ui->filter,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(updateList(QString)));
    QPushButton *button = m_ui->button_box->button(QDialogButtonBox::Apply);
    connect(button, SIGNAL(clicked()), this, SLOT(applyChanges()));

    if (!m_ui->settings_box->layout()) {
        m_ui->settings_box->setLayout(new QGridLayout());
        m_ui->settings_box->layout()->setContentsMargins(2, 9, 2, 9);
    }

    ISettings *settings = new GeneralSettings();
    addItem(settings);
    addItem(new GeneralPluginSettings());

    settings->show();

    load();
    updateList("");
}

SettingsDialog::~SettingsDialog()
{
    save();
    delete m_ui;
}

void SettingsDialog::addItem(ISettings *item)
{
    m_settings.push_back(item);
    m_ui->settings_box->layout()->addWidget(item);
    item->hide();
}

void SettingsDialog::deleteItem(ISettings *item)
{
    for (unsigned int i = 0; i < m_settings.size(); i++) {
        if (m_settings[i] == item) {
            m_settings.erase(m_settings.begin() + i);
        }
    }
}

ISettings *SettingsDialog::findItem(std::string title)
{
    for (unsigned int i = 0; i < m_settings.size(); i++) {
        if (m_settings[i]->title() == title) {
            return m_settings[i];
        }
    }

    return NULL;
}

void SettingsDialog::load()
{
    Logger::getInstance()->debug("SettingsDialog::load()");
    QFile file(SETTINGS_FILE_PATH);
    if (!file.exists()) {
        for (unsigned int i = 0; i < m_settings.size(); i++) {
            ISettings *settings = m_settings[i];
            settings->init();
        }
        return;
    }

    std::string path = SETTINGS_FILE_PATH;
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::getInstance()->error("Failed to read from " + path +
                                     ", error: " + file.errorString().toStdString(), false);
        return;
    }

    for (unsigned int i = 0; i < m_settings.size(); i++) {
        ISettings *settings = m_settings[i];
        file.reset();
        settings->load(file);
    }

    file.close();
}

void SettingsDialog::save()
{
    Logger::getInstance()->debug("SettingsDialog::save()");

    std::string path = SETTINGS_FILE_PATH;
    QFile file(SETTINGS_FILE_PATH);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::getInstance()->error("Failed to write to " + path +
                                     ", error: " + file.errorString().toStdString(), false);
        return;
    }

    QXmlStreamWriter out(&file);

    out.setAutoFormatting(true);
    out.writeStartDocument();

    out.writeStartElement("settings");
    for (unsigned int i = 0; i < m_settings.size(); i++) {
        ISettings *settings = m_settings[i];
        std::string title = settings->title();
        title.erase(remove_if(title.begin(), title.end(), isspace), title.end());
        out.writeStartElement(title.c_str());
        settings->save(out);
        out.writeEndElement();
    }
    out.writeEndElement();

    out.writeEndDocument();
    file.close();
}

void SettingsDialog::change()
{
    QList<QListWidgetItem *> list = m_ui->list->selectedItems();
    if (list.empty()) {
        return;
    }

    for (unsigned int i = 0; i < m_settings.size(); i++) {
        m_settings[i]->hide();
    }

    m_ui->settings_box->setTitle(list[0]->text());
    ISettings *settings = findItem(list[0]->text().toStdString());
    settings->show();
}

void SettingsDialog::updateList(QString text)
{
    m_ui->list->clear();
    for (unsigned int i = 0; i < m_settings.size(); i++) {
        std::string tmp = m_settings[i]->title();
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (tmp.find(text.toLower().toStdString()) != std::string::npos) {
            m_ui->list->addItem(m_settings[i]->title().c_str());
        }
    }
}
