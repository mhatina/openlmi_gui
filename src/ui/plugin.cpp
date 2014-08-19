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
#include <fstream>
#include <QGroupBox>
#include <QToolBar>

bool Engine::IPlugin::isFileEmpty(std::string filename)
{
    Logger::getInstance()->debug("Engine::IPlugin::isFileEmpty(std::string filename)");
    std::ifstream file;
    file.open(filename.c_str());

    bool empty = file.peek() == std::ifstream::traits_type::eof();

    file.close();

    return empty;
}

int Engine::IPlugin::throwAwayChanges()
{
    Logger::getInstance()->debug("Engine::IPlugin::throwAwayChanges()");
    QMessageBox message_box;
    message_box.setWindowTitle("Throw away changes?");
    message_box.setText("Do you really want to throw away changes?");
    message_box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    message_box.setDefaultButton(QMessageBox::Yes);
    return message_box.exec();
}

void Engine::IPlugin::addInstruction(IInstruction *instruction)
{
    Logger::getInstance()->debug("Engine::IPlugin::addInstruction(IInstruction *instruction)");
    if (m_instructions.empty()) {
        std::string hostname = m_client->hostname();
        std::string username = m_client->username();
        m_instructions.push_back(new ConnectInstruction(
                    hostname,
                    username
                    ));
    }

    m_instructions.push_back(instruction);
    emit unsavedChanges(this);
    emit newInstructionText(getInstructionText());
}

void Engine::IPlugin::deleteInstruction(int pos)
{
    Logger::getInstance()->debug("Engine::IPlugin::deleteInstruction(int pos)");
    delete m_instructions[pos];
    m_instructions.erase(m_instructions.begin() + pos);
    emit newInstructionText(getInstructionText());
}

void Engine::IPlugin::insertInstruction(IInstruction *instruction, int pos)
{
    Logger::getInstance()->debug("Engine::IPlugin::insertInstruction(IInstruction *instruction, int pos)");
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
    Logger::getInstance()->debug("Engine::IPlugin::findInstruction(IInstruction::Subject subject, std::string instructionName, int pos)");
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
    m_system_id("")
{
    Logger::getInstance()->debug("Engine::IPlugin::IPlugin()");
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
    Logger::getInstance()->debug("Engine::IPlugin::~IPlugin()");
    delete m_mutex;
}

bool Engine::IPlugin::isFilterShown()
{
    Logger::getInstance()->debug("Engine::IPlugin::isFilterShown()");
    QGroupBox* filter_box = findChild<QGroupBox*>("filter_box");
    if (filter_box != NULL)
        return !filter_box->isHidden();
    return false;
}

bool Engine::IPlugin::isRefreshed()
{
    Logger::getInstance()->debug("Engine::IPlugin::isRefreshed()");
    return m_refreshed;
}

std::string Engine::IPlugin::getSystemId()
{
    return m_system_id;
}

void Engine::IPlugin::applyChanges()
{
    Logger::getInstance()->debug("Engine::IPlugin::applyChanges()");
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
    Logger::getInstance()->debug("Engine::IPlugin::cancelChanges()");
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        delete m_instructions[i];
    m_instructions.clear();
    emit noChanges(this);    
}

void Engine::IPlugin::connectButtons(QToolBar *toolbar)
{
    Logger::getInstance()->debug("Engine::IPlugin::connectButtons(QToolBar *toolbar)");
    QPushButton* button = toolbar->findChild<QPushButton*>("apply_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(apply())
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
    Logger::getInstance()->debug("Engine::IPlugin::refresh(CIMClient *client)");

    if (client == NULL)
        return;

    Logger::getInstance()->info("Refreshing " + getLabel());
    m_client = client;

    emit refreshProgress(0);
//    cancelChanges();
    m_instructions.clear();
    m_data = new std::vector<void *>();    

    boost::thread(boost::bind(&Engine::IPlugin::getData, this, m_data));
}

void Engine::IPlugin::saveScript(std::string filename)
{
    Logger::getInstance()->debug("Engine::IPlugin::saveScript(std::string filename)");
    if (m_instructions.empty())
        return;

    std::ofstream out_file;
    out_file.open(filename.c_str(), std::fstream::out | std::fstream::app);

    unsigned int i;
    bool empty = isFileEmpty(filename);
    if (!empty)
        out_file << "\n";
    for (i = empty ? 0 : 1; i < m_instructions.size(); i++) {
        out_file << m_instructions[i]->toString();
    }

    out_file.close();
}

void Engine::IPlugin::setActive(bool active)
{
    Logger::getInstance()->debug("Engine::IPlugin::setActive(bool active)");
    m_active = active;
}

void Engine::IPlugin::setRefreshed(bool refreshed)
{
    Logger::getInstance()->debug("Engine::IPlugin::setRefreshed(bool refreshed)");
    setPluginEnabled(refreshed);
    m_refreshed = refreshed;    
}

void Engine::IPlugin::apply()
{
    Logger::getInstance()->debug("Engine::IPlugin::apply()");
    if (!m_active)
        return;
    Logger::getInstance()->info("Applying");
    emit refreshProgress(0);
    boost::thread(boost::bind(&Engine::IPlugin::applyChanges, this));
}

void Engine::IPlugin::cancel()
{
    Logger::getInstance()->debug("Engine::IPlugin::cancel()");
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
    Logger::getInstance()->debug("Engine::IPlugin::handleDataFetching(std::vector<void *> *data, std::string error_message)");
    if (data != NULL) {
        setRefreshed(true);
        emit refreshProgress(100);
        fillTab(data);
        delete data;
        Logger::getInstance()->info(getLabel() + ": " + getRefreshInfo());
    } else {
        setRefreshed(false);
        Logger::getInstance()->error(error_message);
        emit refreshProgress(1);
    }
}

void Engine::IPlugin::handleDoneApplying()
{
    Logger::getInstance()->debug("Engine::IPlugin::handleDoneApplying()");
    setRefreshed(false);
    refresh(m_client);
}

void Engine::IPlugin::setPluginEnabled(bool state)
{
    Logger::getInstance()->debug("Engine::IPlugin::setPluginEnabled(bool state)");
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

void Engine::IPlugin::setSystemId(std::string system_id)
{
    m_system_id = system_id;
}

void Engine::IPlugin::showFilter(bool show)
{
    Logger::getInstance()->debug("Engine::IPlugin::showFilter(bool show)");
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
