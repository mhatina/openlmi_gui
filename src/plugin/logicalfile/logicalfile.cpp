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

#include "logicalfile.h"
#include "cimvalue.h"
#include "ui_logicalfile.h"

#include <sstream>

void LogicalFilePlugin::getInfo(std::vector<Pegasus::CIMInstance> *data, std::string name, bool dir)
{
    emit refreshProgress(Engine::NOT_REFRESHED, this);
    if (dir) {

    } else {

    }

    emit refreshProgress(Engine::REFRESHED, this);
    emit writeData(data);
}

void LogicalFilePlugin::writeInfo(std::vector<Pegasus::CIMInstance> *data, std::string name)
{
    delete data;
}

LogicalFilePlugin::LogicalFilePlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_file_cnt(0),
    m_ui(new Ui::LogicalFilePlugin)
{
    m_ui->setupUi(this);
    setPluginEnabled(false);

    m_ui->filter_box->hide();

    connect(
        m_ui->dir_list,
        SIGNAL(itemExpanded(QTreeWidgetItem *)),
        this,
        SLOT(dirExpanded(QTreeWidgetItem *)));
    connect(
        m_ui->dir_list,
        SIGNAL(itemClicked(QTreeWidgetItem *, int)),
        this,
        SLOT(dirClicked(QTreeWidgetItem *)));
}

LogicalFilePlugin::~LogicalFilePlugin()
{
    delete m_ui;
}

std::string LogicalFilePlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string LogicalFilePlugin::getLabel()
{
    return "Logical File";
}

std::string LogicalFilePlugin::getRefreshInfo()
{
    std::stringstream ss;
    bool empty = m_dirs.empty() && m_file_cnt == 0;
    if (!empty) {
        if (!m_dirs.empty()) {
            ss << m_dirs.size() << " directory(ies)";
        }
        if (m_file_cnt != 0) {
            ss << ", " << m_file_cnt << " file(s)";
        }

        ss << " shown";
    } else {
        ss << "Nothing to show";
    }

    return ss.str();
}

void LogicalFilePlugin::clear()
{

}

void LogicalFilePlugin::getData(std::vector<void *> *data)
{
    try {
        Pegasus::Array<Pegasus::CIMObject> directory =
            execQuery(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::String("WQL"),
                Pegasus::String("SELECT * FROM LMI_UnixDirectory WHERE Name = \"/\""));

        for (unsigned int i = 0; i < directory.size(); i++) {
            data->push_back(new Pegasus::CIMInstance(directory[i]));
        }

    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, std::string(ex.getMessage().getCString()));
        return;
    }

    emit doneFetchingData(data);
}

void LogicalFilePlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    unsigned int cnt = data->size();
    try {
        for (unsigned int i = 0; i < cnt; i++) {
            Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[i]);

            QTreeWidgetItem *item;
            if (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_UnixDirectory") {
                item = new QTreeWidgetItem();
                std::string rights = CIMValue::get_property_value(*instance, "Readable") == "True" ? "r" : "-";
                rights += CIMValue::get_property_value(*instance, "Writeable") == "True" ? "w" : "-";
                rights += CIMValue::get_property_value(*instance, "Executable") == "True" ? "x" : "-";
                item->setText(0, rights.c_str());
                item->setText(1, CIMValue::get_property_value(*instance, "Name").c_str());
                item->setText(2, CIMValue::get_property_value(*instance, "FileSize").c_str());
                item->setText(3, CIMValue::get_property_value(*instance, "LastModified").c_str());
                m_ui->dir_list->addTopLevelItem(item);
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    for (unsigned int i = 0; i < cnt; i++) {
        delete ((Pegasus::CIMInstance *) (*data)[i]);
    }

    m_changes_enabled = true;
}

void LogicalFilePlugin::dirClicked(QTreeWidgetItem *item)
{
    // TODO get files in dir
}

void LogicalFilePlugin::dirExpanded(QTreeWidgetItem *item)
{
    // TODO get dirs in dir
}

Q_EXPORT_PLUGIN2(logicalFile, LogicalFilePlugin)
