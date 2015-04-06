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
#include "cimvalue.h"
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
    QMessageBox message_box(this);
    message_box.setWindowTitle("Throw away changes?");
    message_box.setText("Do you really want to throw away changes?");
    message_box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    message_box.setDefaultButton(QMessageBox::Yes);
    return message_box.exec();
}

void Engine::IPlugin::addInstruction(IInstruction *instruction)
{
    Logger::getInstance()->debug("Engine::IPlugin::addInstruction(IInstruction *instruction)");
    if (instruction == NULL) {
        return;
    }

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
    if (m_instructions[pos] != NULL) {
            delete m_instructions[pos];
            m_instructions[pos] = NULL;
        }
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
    connect(instruction, SIGNAL(error(std::string)), this,
            SLOT(handleError(std::string)));
    emit unsavedChanges(this);
    emit newInstructionText(getInstructionText());
}

int Engine::IPlugin::findInstruction(IInstruction::Subject subject,
                                     std::string instructionName, int pos)
{
    Logger::getInstance()->debug("Engine::IPlugin::findInstruction(IInstruction::Subject subject, std::string instructionName, int pos)");
    std::vector<IInstruction *>::iterator it;
    unsigned int i = pos;
    if (i > m_instructions.size()) {
        return -1;
    }
    for (it = m_instructions.begin() + pos; it != m_instructions.end(); it++) {
        bool found = true;

        found &= (*it)->getSubject() == subject;

        if (!instructionName.empty()) {
            found &= (*it)->getInstructionName() == instructionName;
        }

        if (!found) {
            i++;
            continue;
        }

        return i;
    }

    return -1;
}

Engine::IPlugin::IPlugin() :
    m_refreshing(false),
    m_active(true),
    m_changes_enabled(false),
    m_refreshed(false),
    m_client(NULL),
    m_system_id("")
{
    Logger::getInstance()->debug("Engine::IPlugin::IPlugin()");
    qRegisterMetaType<std::string>("std::string");
    connect(
        this,
        SIGNAL(doneFetchingData(std::vector<void*>*,bool,std::string)),
        this,
        SLOT(handleDataFetching(std::vector<void*>*,bool,std::string)));
    connect(
        this,
        SIGNAL(doneApplying()),
        this,
        SLOT(handleDoneApplying()));
}

Engine::IPlugin::~IPlugin()
{
    Logger::getInstance()->debug("Engine::IPlugin::~IPlugin()");
}

Pegasus::Array<Pegasus::CIMObject> Engine::IPlugin::associators(
        const Pegasus::CIMNamespaceName &nameSpace,
        const Pegasus::CIMObjectPath &objectName,
        const Pegasus::CIMName &assocClass,
        const Pegasus::CIMName &resultClass,
        const Pegasus::String &role,
        const Pegasus::String &resultRole,
        Pegasus::Boolean includeQualifiers,
        Pegasus::Boolean includeClassOrigin,
        const Pegasus::CIMPropertyList &propertyList)
{
    boost::this_thread::interruption_point();
    Pegasus::Array<Pegasus::CIMObject> array;
    m_mutex->lock();
    try {
        array = m_client->associators(
                    nameSpace,
                    objectName,
                    assocClass,
                    resultClass,
                    role,
                    resultRole,
                    includeQualifiers,
                    includeClassOrigin,
                    propertyList);
    } catch (Pegasus::Exception &ex) {
        m_mutex->unlock();
        throw ex;
    }
    m_mutex->unlock();

    boost::this_thread::interruption_point();
    return array;
}

Pegasus::Array<Pegasus::CIMInstance> Engine::IPlugin::enumerateInstances(
    const Pegasus::CIMNamespaceName &nameSpace,
    const Pegasus::CIMName &className,
    Pegasus::Boolean deepInheritance,
    Pegasus::Boolean localOnly,
    Pegasus::Boolean includeQualifiers,
    Pegasus::Boolean includeClassOrigin,
    const Pegasus::CIMPropertyList &propertyList)
{
    boost::this_thread::interruption_point();
    Pegasus::Array<Pegasus::CIMInstance> array;
    m_mutex->lock();
    try {
        array = m_client->enumerateInstances(
                    nameSpace,
                    className,
                    deepInheritance,
                    localOnly,
                    includeQualifiers,
                    includeClassOrigin,
                    propertyList);
    } catch (Pegasus::Exception &ex) {
        m_mutex->unlock();
        throw ex;
    }
    m_mutex->unlock();

    boost::this_thread::interruption_point();
    return array;
}

Pegasus::Array<Pegasus::CIMObject> Engine::IPlugin::execQuery(
        const Pegasus::CIMNamespaceName &nameSpace,
        const Pegasus::String &queryLanguage,
        const Pegasus::String &query)
{
    boost::this_thread::interruption_point();
    Pegasus::Array<Pegasus::CIMObject> array;
    m_mutex->lock();
    try {
        array = m_client->execQuery(
                    nameSpace,
                    queryLanguage,
                    query);
    } catch (Pegasus::Exception &ex) {
        m_mutex->unlock();
        throw ex;
    }
    m_mutex->unlock();

    boost::this_thread::interruption_point();
    return array;
}

Engine::filter_status Engine::IPlugin::isFilterAvailable()
{
    QGroupBox *filter_box = findChild<QGroupBox *>("filter_box");
    if (filter_box == NULL) {
        return FILTER_ERROR;
    }

    QObjectList list = filter_box->children();
    if (list.empty() || list.size() == 1) {
        return FILTER_NOT_AVAILABLE;
    }

    return FILTER_AVAILABLE;
}

bool Engine::IPlugin::isFilterShown()
{
    Logger::getInstance()->debug("Engine::IPlugin::isFilterShown()");
    QGroupBox *filter_box = findChild<QGroupBox *>("filter_box");
    if (filter_box != NULL) {
        return !filter_box->isHidden();
    }
    return false;
}

bool Engine::IPlugin::isRefreshed()
{
    Logger::getInstance()->debug("Engine::IPlugin::isRefreshed()");
    return m_refreshed;
}

bool Engine::IPlugin::showFilter(bool show)
{
    Logger::getInstance()->debug("Engine::IPlugin::showFilter(bool show)");

    QGroupBox *filter_box = findChild<QGroupBox *>("filter_box");
    switch (isFilterAvailable()) {
    case FILTER_AVAILABLE:
        filter_box->setVisible(show);
        return show;
    case FILTER_NOT_AVAILABLE:
        filter_box->setVisible(false);
        return false;
    case FILTER_ERROR:
        Logger::getInstance()->error("Unable to show/hide filter!");
        return false;
    }

    return true;
}

std::string Engine::IPlugin::getSystemId()
{
    Logger::getInstance()->debug("Engine::IPlugin::getSystemId()");
    return m_system_id;
}

void Engine::IPlugin::applyChanges()
{
    Logger::getInstance()->debug("Engine::IPlugin::applyChanges()");
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->run();
        if (m_instructions[i] != NULL) {
                delete m_instructions[i];
                m_instructions[i] = NULL;
            }
    }
    m_instructions.clear();
    emit noChanges(this);
    emit doneApplying();
}

void Engine::IPlugin::cancelChanges()
{
    Logger::getInstance()->debug("Engine::IPlugin::cancelChanges()");
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        if (m_instructions[i] != NULL) {
                delete m_instructions[i];
                m_instructions[i] = NULL;
            }
    }
    m_instructions.clear();
    emit noChanges(this);
}

void Engine::IPlugin::refresh(CIMClient *client)
{
    Logger::getInstance()->debug("Engine::IPlugin::refresh(CIMClient *client)");

    if (client == NULL) {
        emit refreshProgress(Engine::ERROR, this);
        return;
    }

    m_client = client;
    m_data = new std::vector<void *>();
    setRefreshing(true);

    cancelChanges();
    clear();

    m_refresh_thread = boost::thread(boost::bind(&Engine::IPlugin::getData, this,
                                     m_data));
}

void Engine::IPlugin::saveScript(std::string filename)
{
    Logger::getInstance()->debug("Engine::IPlugin::saveScript(std::string filename)");
    if (m_instructions.empty()) {
        return;
    }

    std::ofstream out_file;
    out_file.open(filename.c_str(), std::fstream::out | std::fstream::app);

    unsigned int i;
    bool empty = isFileEmpty(filename);
    if (!empty) {
        out_file << "\n";
    }
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

void Engine::IPlugin::setMutex(QMutex *mutex)
{
    m_mutex = mutex;
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
    if (!m_active) {
        return;
    }
    Logger::getInstance()->info("Applying");
    setRefreshed(false);
    emit refreshProgress(Engine::NOT_REFRESHED, this);
    boost::thread(boost::bind(&Engine::IPlugin::applyChanges, this));
}

void Engine::IPlugin::cancel()
{
    Logger::getInstance()->debug("Engine::IPlugin::cancel()");
    if (!m_active) {
        return;
    }
    if (m_instructions.empty()) {
        return;
    }

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

void Engine::IPlugin::handleDataFetching(std::vector<void *> *data, bool still_refreshing,
        std::string error_message)
{
    Logger::getInstance()->debug("Engine::IPlugin::handleDataFetching(std::vector<void *> *data, std::string error_message)");
    if (!still_refreshing) {
        m_refresh_thread.join();
    }

    if (!error_message.empty()) {
        if (error_message.find("Unauthorized") != std::string::npos) {
            Logger::getInstance()->error("Wrong username or password!");
            emit deletePasswd();
        } else {
            setRefreshed(false);
            emit refreshProgress(Engine::ERROR, this);
            Logger::getInstance()->error(error_message);
        }
    } else if (data != NULL) {        
        setRefreshed(true);
        fillTab(data);
        if (!still_refreshing) {
            setRefreshing(false);
            emit refreshProgress(Engine::REFRESHED, this);
            Logger::getInstance()->info(getRefreshInfo());
        }        
        if (data != NULL) {
                delete data;
                data = NULL;
            }
    }
}

void Engine::IPlugin::handleDoneApplying()
{
    Logger::getInstance()->debug("Engine::IPlugin::handleDoneApplying()");    
    refresh(m_client);
}

void Engine::IPlugin::setPluginEnabled(bool state)
{
    Logger::getInstance()->debug("Engine::IPlugin::setPluginEnabled(bool state)");
    setEnabled(true);
    QList<QWidget *> children = findChildren<QWidget *>();
    int cnt = children.size();

    for (int i = 0; i < cnt; i++) {
        children[i]->setEnabled(state);
    }

    QGroupBox *filter_box = findChild<QGroupBox *>("filter_box");

    if (filter_box != NULL) {
        filter_box->setEnabled(true);
        QList<QWidget *> filter_box_children = filter_box->findChildren<QWidget *>();
        int filter_box_children_cnt = filter_box_children.size();

        for (int i = 0; i < filter_box_children_cnt; i++) {
            filter_box_children[i]->setEnabled(true);
        }
    }
}

void Engine::IPlugin::stopRefresh()
{
    Logger::getInstance()->debug("Engine::IPlugin::stopRefresh()");
    m_refresh_thread.interrupt();
    setRefreshing(false);
    emit refreshProgress(Engine::STOP_REFRESH, this);
}

bool Engine::IPlugin::isRefreshing() const
{
    return m_refreshing;
}

void Engine::IPlugin::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
}
