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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "instructions/instruction.h"
#include "kernel.h"
#include "lmiwbem_client.h"
#include "logger.h"
#include "widgets/providerwidget.h"

#include <boost/thread.hpp>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QWidget>

namespace Engine
{

/**
 * @brief struct representation of property of LMI_{Account/Group/Service}
 */
typedef struct {
    const char *property; /**< @brief Actual name of property */
    bool enabled; /**< @brief Whether the property is editable or not */
} property;

typedef struct {
    QString name;
    QString tooltip;
} header_item;

class Kernel;

/**
 * @brief The IPlugin class.
 *
 * Class that represent a OpenLMI provider
 */
class IPlugin : public QWidget
{
    Q_OBJECT

private:
    bool m_stop_refresh;
    bool isFileEmpty(std::string filename);

protected:
    bool m_active; /**< @brief whether provider is active and is being displayed */
    bool m_changes_enabled; /**< @brief whether any changes are enabled */
    bool m_refreshed; /**< @brief represent refresh state of provider */
    bool m_still_refreshing;
    boost::thread m_refresh_thread;
    CIMClient *m_client; /**< @brief see CIMClient */
    QMutex *m_mutex; /**< @brief Qt mutex */
    std::string m_system_id;
    std::vector<IInstruction *>
    m_instructions; /**< @brief vector where all instructions are stored*/
    std::vector<void *>
    *m_data; /**< @brief vector where data from provider are stored */

    /**
     * @brief Dialog with question for user whether throw away changes
     * @return QMessageBox::Yes / QMessageBox::No
     */
    int throwAwayChanges();

    /**
     * @brief Add new instruction to end of m_instructions
     * @param instruction -- see IInstruction
     */
    void addInstruction(IInstruction *instruction);

    void deleteInstruction(int pos);

    /**
     * @brief Insert new instruction to @ref pos
     * @param instruction -- see IInstruction
     * @param pos -- position where instruction should be inserted
     */
    void insertInstruction(IInstruction *instruction, int pos);

    /**
     * @brief findInstruction -- find all occurences of instructions that fulfill requirements
     * @return position of instruction
     */
    int findInstruction(IInstruction::Subject subject,
                        std::string instructionName = "", int pos = 0);

public:
    /**
     * @brief Constructor
     */
    IPlugin();
    /**
     * @brief Virtual destructor
     */
    virtual ~IPlugin();

    Pegasus::Array<Pegasus::CIMInstance> enumerateInstances(
        const Pegasus::CIMNamespaceName &nameSpace,
        const Pegasus::CIMName &className,
        Pegasus::Boolean deepInheritance = true,
        Pegasus::Boolean localOnly = true,
        Pegasus::Boolean includeQualifiers = false,
        Pegasus::Boolean includeClassOrigin = false,
        const Pegasus::CIMPropertyList &propertyList = Pegasus::CIMPropertyList());
    /**
     * @brief isFilterShown
     * @return  return true if filter is displayed, otherwise false
     */
    bool isFilterShown();
    /**
     * @brief Refresh state of provider
     * @return refresh state
     */
    bool isRefreshed();
    /**
     * @brief Method for displaying filter
     * @param show - if true filter is displayed
     */
    bool showFilter(bool show);
    std::string getSystemId();
    /**
     * @brief Virtual method for getting LMIShell code of all instructions
     * @return LMIShell code
     */
    virtual std::string getInstructionText() = 0;
    /**
     * @brief Label of provider
     * @return label
     */
    virtual std::string getLabel() = 0;
    virtual std::string getRefreshInfo() = 0;
    virtual void clear() = 0;
    /**
     * @brief Display all data
     * @param data
     */
    virtual void fillTab(std::vector<void *> *data) = 0;
    /**
     * @brief Method for fetching data
     * @param data
     */
    virtual void getData(std::vector<void *> *data) = 0;
    /**
     * @brief setPluginEnabled -- enable/disable plugin
     * @param state
     */
    void setPluginEnabled(bool state);
    /**
     * @brief Apply all changes
     *
     * Should be run in new thread.
     */
    void applyChanges();
    /**
     * @brief Cancel all changes
     */
    void cancelChanges();
    /**
     * @brief Connect buttons' signals with handlers
     * @param toolbar -- contains buttons
     */
    void connectButtons(QToolBar *toolbar);
    /**
     * @brief Refresh whole provider
     * @param client -- see CIMClient
     */
    void refresh(CIMClient *client);
    /**
     * @brief Save LMIShell code from instructions to file
     *
     * Form can be defined later in provider.
     */
    void saveScript(std::string filename);
    /**
     * @brief Setter
     * @param active -- whether provider is active
     */
    void setActive(bool active);
    void setMutex(QMutex *mutex);
    /**
     * @brief Setter
     * @param refreshed -- refresh state
     */
    void setRefreshed(bool refreshed);

    void stopRefresh();

protected slots:
    /**
     * @brief Apply all changes.
     */
    void apply();
    /**
     * @brief Cancel all changes
     *
     * Dialog with "Are you sure?" question is displayed.
     */
    void cancel();
    /**
     * @brief Process fetched data from @ref doneFetchingData
     * @param data
     * @param error_message -- if something occures when fetching data, error_message is filled with error message
     */
    void handleDataFetching(std::vector<void *> *data, std::string error_message);
    /**
     * @brief Process applying itself (refresh provider, set to refreshed, ...)
     */
    void handleDoneApplying();

signals:
    /**
     * @brief Emitted when all changes are applied
     */
    void doneApplying();
    /**
     * @brief Emitted when data from provider are fetched
     * @param data
     * @param error_message -- if something occures when fetching data, error_message is filled with error message
     */
    void doneFetchingData(std::vector<void *> *data,
                          std::string error_message = std::string());
    /**
     * @brief @brief Emitted when new instruction is added/inserted
     * @param text -- text of new instruction
     */
    void newInstructionText(std::string text);
    /**
     * @brief emitted when there are no changes to be applied/saved
     * @param plugin -- representation of provider
     */
    void noChanges(IPlugin *plugin);
    /**
     * @brief Signal to inform about progress of refreshing
     * @param progress -- progress state
     */
    void refreshProgress(int progress, IPlugin *plugin);
    /**
     * @brief emitted when there are some changes to be applied/saved
     * @param plugin -- representation of provider
     */
    void unsavedChanges(IPlugin *plugin);
};
} // namespace Engine

Q_DECLARE_INTERFACE(Engine::IPlugin, "com.redhat.openlmi.src.ui.plugin")

#endif // PLUGIN_H
