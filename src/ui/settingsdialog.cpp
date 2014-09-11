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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);
    load();    

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

    m_settings_items.push_back("General");
    m_settings_items.push_back("Plugin general");
    updateList("");
}

SettingsDialog::~SettingsDialog()
{
    save();
    delete m_ui;
}

void SettingsDialog::load()
{

}

void SettingsDialog::save()
{

}

void SettingsDialog::change()
{
    QList<QListWidgetItem*> list = m_ui->list->selectedItems();
    if (list.empty())
        return;

    m_ui->settings_box->setTitle(list[0]->text());
    if (list[0]->text() == "General") {

    } else if (list[0]->text() == "Plugin general") {

    }
}

void SettingsDialog::updateList(QString text)
{
    m_ui->list->clear();
    for (unsigned int i = 0; i < m_settings_items.size(); i++) {
        std::string tmp = m_settings_items[i];
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (tmp.find(text.toLower().toStdString()) != std::string::npos)
            m_ui->list->addItem(m_settings_items[i].c_str());
    }
}
