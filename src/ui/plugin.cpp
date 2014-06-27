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

#include "instructions/connectinstruction.h"
#include "lmiwbem_value.h"
#include "plugin.h"

#include <boost/thread.hpp>
#include <QGroupBox>
#include <QToolBar>

void Engine::IPlugin::setPluginEnabled(bool state)
{
    setEnabled(true);
    QList<QWidget*> children = findChildren<QWidget*>();
    int cnt = children.size();

    for (int i = 0; i < cnt; i++)
        children[i]->setEnabled(state);

    QGroupBox* filter_box = findChild<QGroupBox*>("filter_box");

    if (filter_box != NULL) {
        filter_box->setEnabled(true);
        QList<QWidget*> filter_box_children = filter_box->findChildren<QWidget*>();
        int filter_box_children_cnt = filter_box_children.size();

        for (int i = 0; i < filter_box_children_cnt; i++)
            filter_box_children[i]->setEnabled(true);
    }
}

int Engine::IPlugin::throwAwayChanges()
{
    QMessageBox message_box;
    message_box.setWindowTitle("Throw away changes?");
    message_box.setText("Do you really want to throw away changes?");
    message_box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    message_box.setDefaultButton(QMessageBox::Yes);
    return message_box.exec();
}

std::string Engine::IPlugin::getPath()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    return dialog.getExistingDirectory().toStdString();
}

std::string Engine::IPlugin::getPropertyOfInstance(Pegasus::CIMInstance instance,
                                                         std::string propertyName, Pegasus::CIMProperty *property)
{
    Pegasus::Uint32 propIndex = instance.findProperty(Pegasus::CIMName(propertyName.c_str()));
    if (propIndex == Pegasus::PEG_NOT_FOUND) {
        Logger::getInstance()->error("property " + propertyName + " not found");
        return "";
    }
    Pegasus::CIMProperty prop = instance.getProperty(propIndex);
    if (property != NULL) {
        *property = prop;
    }
    Pegasus::CIMValue value = prop.getValue();
    return CIMValue::to_std_string(value);
}

void Engine::IPlugin::addInstruction(IInstruction *instruction)
{
    if (m_instructions.empty()) {
        std::string hostname = m_client->hostname();
        std::string username = m_client->username();
        m_instructions.push_back(new ConnectInstruction(
                    hostname,
                    username
                    ));
    }

    m_instructions.push_back(instruction);
    connect(instruction, SIGNAL(error(std::string)), this, SLOT(handleError(std::string)));
    emit unsavedChanges(this);
    emit newInstructionText(getInstructionText());
}

void Engine::IPlugin::deleteInstruction(int pos)
{
    delete m_instructions[pos];
    m_instructions.erase(m_instructions.begin() + pos);
    emit newInstructionText(getInstructionText());
}

void Engine::IPlugin::insertInstruction(IInstruction *instruction, int pos)
{
    m_instructions.insert(
                m_instructions.begin() + pos,
                1,
                instruction
            );
    connect(instruction, SIGNAL(error(std::string)), this, SLOT(handleError(std::string)));
    emit unsavedChanges(this);
    emit newInstructionText(getInstructionText());
}

int Engine::IPlugin::findInstruction(IInstruction::Subject subject, std::string instructionName, int pos)
{
    std::vector<IInstruction*>::iterator it;
    unsigned int i = pos;
    if (i > m_instructions.size())
        return -1;
    for (it = m_instructions.begin() + pos; it != m_instructions.end(); it++) {
        bool found = true;

        found &= (*it)->getSubject() == subject;

        if (!instructionName.empty())
            found &= (*it)->getInstructionName() == instructionName;

        if (!found) {
            i++;
            continue;
        }

        return i;
    }

    return -1;
}

Engine::IPlugin::IPlugin() :
    m_active(true),
    m_changes_enabled(false),
    m_refreshed(false),
    m_client(NULL),
    m_mutex(new QMutex(QMutex::Recursive)),
    m_save_dir_path(""),
    m_save_dir_path_backup("")
{
    qRegisterMetaType<std::string>("std::string");
    connect(
        this,
        SIGNAL(doneFetchingData(std::vector<void*>*,std::string)),
        this,
        SLOT(handleDataFetching(std::vector<void*>*,std::string)));
    connect(
        this,
        SIGNAL(doneApplying()),
        this,
        SLOT(handleDoneApplying()));
}

Engine::IPlugin::~IPlugin()
{    
    delete m_mutex;
}

bool Engine::IPlugin::isFilterShown()
{
    QGroupBox* filter_box = findChild<QGroupBox*>("filter_box");
    if (filter_box != NULL)
        return !filter_box->isHidden();
    return false;
}

bool Engine::IPlugin::isRefreshed()
{
    return m_refreshed;
}

void Engine::IPlugin::applyChanges()
{
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->run();
        delete m_instructions[i];
    }
    m_instructions.clear();
    emit noChanges(this);
    emit doneApplying();
}

void Engine::IPlugin::cancelChanges()
{
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        delete m_instructions[i];
    m_instructions.clear();
    emit noChanges(this);    
}

void Engine::IPlugin::connectButtons(QToolBar *toolbar)
{
    QPushButton* button = toolbar->findChild<QPushButton*>("apply_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(apply())
        );
    button = toolbar->findChild<QPushButton*>("save_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(save())
        );
    button = toolbar->findChild<QPushButton*>("save_as_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(saveAs())
        );
    button = toolbar->findChild<QPushButton*>("cancel_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(cancel())
        );
}

void Engine::IPlugin::refresh(CIMClient *client)
{
    Logger::getInstance()->debug("refresh");    

    if (client == NULL)
        return;

    Logger::getInstance()->info("Refreshing " + getLabel());
    m_client = client;

    emit refreshProgress(0);
//    cancelChanges();
    m_instructions.clear();
    m_data = new std::vector<void *>();
    setRefreshed(true);

    boost::thread(boost::bind(&Engine::IPlugin::getData, this, m_data));
}

void Engine::IPlugin::setActive(bool active)
{
    m_active = active;
}

void Engine::IPlugin::setRefreshed(bool refreshed)
{
    setPluginEnabled(refreshed);
    m_refreshed = refreshed;    
}

void Engine::IPlugin::apply()
{
    if (!m_active)
        return;
    Logger::getInstance()->info("Applying");
    emit refreshProgress(0);
    boost::thread(boost::bind(&Engine::IPlugin::applyChanges, this));
}

void Engine::IPlugin::cancel()
{
    if (!m_active)
        return;
    if (m_instructions.empty())
        return;

    int ret = throwAwayChanges();

    switch (ret) {
    case QMessageBox::Yes:
        setRefreshed(false);
        refresh(m_client);
        cancelChanges();
        emit newInstructionText("");
        break;
    case QMessageBox::No:
    default:
        return;
    }
}

void Engine::IPlugin::handleDataFetching(std::vector<void *> *data, std::string error_message)
{
    if (data != NULL) {
        fillTab(data);
        setRefreshed(true);
        delete data;
        emit refreshProgress(100);
    } else {
        setRefreshed(false);
        Logger::getInstance()->error(error_message);
        emit refreshProgress(1);
    }
}

void Engine::IPlugin::handleDoneApplying()
{
    setRefreshed(false);
    refresh(m_client);
}

void Engine::IPlugin::handleError(std::string message)
{
    Logger::getInstance()->error(message);
}

void Engine::IPlugin::save()
{
    if (!m_active)
        return;

    if (m_save_dir_path.empty())
        m_save_dir_path = getPath();

    if (m_save_dir_path.empty()) {
        m_save_dir_path = m_save_dir_path_backup;
        return;
    }

    generateCode();
}

void Engine::IPlugin::saveAs()
{
    m_save_dir_path_backup = m_save_dir_path;
    m_save_dir_path.clear();
    save();
}


void Engine::IPlugin::showFilter(bool show)
{
    QGroupBox* filter_box = findChild<QGroupBox*>("filter_box");
    if (filter_box == NULL) {
        Logger::getInstance()->error("Unable to show/hide filter!");
        return;
    }

    if (show) {
        filter_box->show();
    } else {
        filter_box->hide();
    }
}
