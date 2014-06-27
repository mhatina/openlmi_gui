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

#include "eventlog.h"
#include "kernel.h"
#include "logger.h"

#include <boost/thread.hpp>
#include <gnome-keyring-1/gnome-keyring.h>
#include <Pegasus/Common/Array.h>
#include <QStatusBar>
#include <QToolBar>

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;
#define OPENLMI_KEYRING_DEFAULT "openlmi"

Engine::Kernel::Kernel() :
    m_refreshEnabled(true),
    m_mutex(new QMutex()),
    m_bar(new QProgressBar(m_main_window.getProviderWidget()))
{
    QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("refresh_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(refresh())
        );
    button = m_main_window.getToolbar()->findChild<QPushButton*>("shutdown_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(shutdownPc()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("reboot_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(rebootPc()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("edit_button");
    connect(
        button,
        SIGNAL(toggled(bool)),
        this,
        SLOT(setEditState(bool)));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(deletePasswd()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("show_code_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showCodeDialog()));
    button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(showFilter()));
    connect(
        m_main_window.getPcTreeWidget(),
        SIGNAL(removed(std::string)),
        this,
        SLOT(deletePasswd(std::string)));
    connect(
        m_main_window.getProviderWidget()->getTabWidget(),
        SIGNAL(currentChanged(int)),
        this,
        SLOT(setActivePlugin(int)));
    connect(
        m_main_window.getPcTreeWidget()->getTree(),
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(enableSpecialButtons()));
    qRegisterMetaType<PowerStateValues::POWER_VALUES>("PowerStateValues::POWER_VALUES");
    connect(
        this,
        SIGNAL(doneConnecting(CIMClient*,PowerStateValues::POWER_VALUES)),
        this,
        SLOT(handleConnecting(CIMClient*,PowerStateValues::POWER_VALUES)));
    connect(
        this,
        SIGNAL(error(std::string)),
        this,
        SLOT(handleError(std::string)));
    connect(
        this,
        SIGNAL(authenticate(PowerStateValues::POWER_VALUES)),
        this,
        SLOT(handleAuthentication(PowerStateValues::POWER_VALUES)));
    connect(
        m_main_window.getResetPasswdStorageAction(),
        SIGNAL(triggered()),
        this,
        SLOT(resetKeyring()));
    m_bar->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_code_dialog.setTitle("LMIShell Code");
    createKeyring();

    m_event_log.setConnectionStorage(&m_connections);
    m_event_log.setPCTree(m_main_window.getPcTreeWidget()->getTree());
}

Engine::Kernel::~Kernel()
{
    Logger::removeInstance();
    delete m_mutex;
    delete m_bar;
}

int Engine::Kernel::getIndexOfTab(std::string name)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    for (int i = 0; i < tab->count(); i++) {
       if (tab->tabText(i).contains(name.c_str()))
           return i;
    }

    return -1;
}

void Engine::Kernel::createKeyring()
{
    GnomeKeyringResult res = gnome_keyring_create_sync(OPENLMI_KEYRING_DEFAULT, NULL);
    if (res != GNOME_KEYRING_RESULT_OK && res != GNOME_KEYRING_RESULT_KEYRING_ALREADY_EXISTS) {
        Logger::getInstance()->error("Cannot create " + std::string(OPENLMI_KEYRING_DEFAULT) + " keyring");
        exit(EXIT_FAILURE);
    }
}

void Engine::Kernel::setButtonsEnabled(bool state, bool refresh_button)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    bool refreshed = plugin->isRefreshed();

    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("apply_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("cancel_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("save_as_button"))->setEnabled(state & refreshed);
    if (refresh_button) {
        enableSpecialButtons(state);
    }
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("shutdown_button"))->setEnabled(state & refreshed);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("reboot_button"))->setEnabled(state & refreshed);
}

void Engine::Kernel::setPowerState(CIMClient *client, PowerStateValues::POWER_VALUES power_state)
{
    Pegasus::CIMObjectPath power_inst_name;
    try {
        power_inst_name = client->enumerateInstanceNames(
            Pegasus::CIMNamespaceName("root/cimv2"),
            Pegasus::CIMName("LMI_PowerManagementService")
            )[0];
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }

    Pegasus::Array<Pegasus::CIMParamValue> in_param;
    Pegasus::Array<Pegasus::CIMParamValue> out_param;

    in_param.append(Pegasus::CIMParamValue(
                        Pegasus::String("PowerState"),
                        Pegasus::CIMValue(
                            Pegasus::Uint16(power_state)
                            )
                        ));
    try {
        client->invokeMethod(
            Pegasus::CIMNamespaceName("root/cimv2"),
            power_inst_name,
            Pegasus::CIMName("RequestPowerStateChange"),
            in_param,
            out_param
            );
    } catch (const Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }
}

void Engine::Kernel::getConnection(PowerStateValues::POWER_VALUES state)
{
    Logger::getInstance()->info("Connecting...");
    QTreeWidgetItem* item = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->text(0).toStdString();

    switch (getSilentConnection(ip)) {
    case 0:
        emit doneConnecting(m_connections[ip], state);
        break;
    case 1:
        emit authenticate(state);
        break;
    case -1:
        // error already displayed
        break;
    }

}

void Engine::Kernel::loadPlugin()
{
    QDir plugins_dir(qApp->applicationDirPath());
    plugins_dir.cd(PLUGIN_PATH);

    foreach (QString file_name, plugins_dir.entryList(QDir::Files)) {
        QPluginLoader plugin_loader(plugins_dir.absoluteFilePath(file_name));
        QObject *plugin = plugin_loader.instance();
        IPlugin *loaded_plugin = NULL;
        if (plugin && (loaded_plugin = qobject_cast<IPlugin*>(plugin))) {
            if (m_loaded_plugins.find(file_name.toStdString()) == m_loaded_plugins.end()) {
                Logger::getInstance()->debug("Loaded: " + loaded_plugin->getLabel(), true);
                m_loaded_plugins[file_name.toStdString()] = loaded_plugin;

                loaded_plugin->connectButtons(m_main_window.getToolbar());
                connect(loaded_plugin, SIGNAL(unsavedChanges(IPlugin*)), this, SLOT(setPluginUnsavedChanges(IPlugin*)));
                connect(loaded_plugin, SIGNAL(noChanges(IPlugin*)), this, SLOT(setPluginNoChanges(IPlugin*)));
                connect(loaded_plugin, SIGNAL(refreshProgress(int)), this, SLOT(handleProgressState(int)));
                connect(loaded_plugin, SIGNAL(newInstructionText(std::string)), this, SLOT(handleInstructionText(std::string)));

                m_main_window.getProviderWidget()->getTabWidget()->addTab(loaded_plugin, loaded_plugin->getLabel().c_str());
            }
        } else {
            Logger::getInstance()->error(plugin_loader.errorString().toStdString());
        }
    }
}

void Engine::Kernel::showMainWindow()
{
    loadPlugin();
    setActivePlugin(0);
    m_main_window.show();
}

int Engine::Kernel::getSilentConnection(std::string ip)
{
    if (m_connections.find(ip) == m_connections.end()) {
        CIMClient *client = NULL;
        GnomeKeyringAttributeList *list = gnome_keyring_attribute_list_new();
        GList *res_list;

        gnome_keyring_attribute_list_append_string(list, "server", ip.c_str());

        GnomeKeyringResult res = gnome_keyring_find_items_sync(
                    GNOME_KEYRING_ITEM_NETWORK_PASSWORD,
                    list,
                    &res_list
                    );

        if (res == GNOME_KEYRING_RESULT_NO_MATCH) {
            gnome_keyring_found_list_free(res_list);
            return 1;
        } else if (res != GNOME_KEYRING_RESULT_OK) {
            emit error("Cannot get username and password");
            gnome_keyring_found_list_free(res_list);
            return -1;
        }

        std::string username;
        GnomeKeyringFound *keyring = ((GnomeKeyringFound*) res_list->data);
        guint cnt = g_array_get_element_size(keyring->attributes);
        for (guint i = 0; i < cnt; i++) {
            GnomeKeyringAttribute tmp;
            if (strcmp((tmp = g_array_index(keyring->attributes, GnomeKeyringAttribute, i)).name, "user") == 0) {
                username = gnome_keyring_attribute_get_string(&tmp);
                break;
            }
        }

        gchar* passwd;
        gnome_keyring_find_password_sync(
                    GNOME_KEYRING_NETWORK_PASSWORD,
                    &passwd,
                    "user", username.c_str(),
                    "server", ip.c_str(),
                    NULL
                    );
        gnome_keyring_found_list_free(res_list);

        client = new CIMClient();
        try {
            client->connect(ip, 5988, false, username, passwd);
            m_connections[ip] = client;
            return 0;
        } catch (Pegasus::Exception &ex) {
            emit error(std::string(ex.getMessage().getCString()));
            return -1;
        }
    }

    return 0;
}

void Engine::Kernel::deletePasswd()
{
    std::string id = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]->text(0).toStdString();
    deletePasswd(id);
}

void Engine::Kernel::deletePasswd(std::string id)
{
    GnomeKeyringResult res = gnome_keyring_delete_password_sync(
                GNOME_KEYRING_NETWORK_PASSWORD,
                "server", id.c_str(),
                NULL
                );

    if (res != GNOME_KEYRING_RESULT_OK)
        Logger::getInstance()->error("Cannot delete password");
    else
        Logger::getInstance()->info("Password deleted");
}


void Engine::Kernel::enableSpecialButtons()
{
    enableSpecialButtons(true);
}

void Engine::Kernel::enableSpecialButtons(bool state)
{
    QPushButton *button = m_main_window.findChild<QPushButton*>("refresh_button");
    QList<QTreeWidgetItem*> list = m_main_window.getPcTreeWidget()->getTree()->selectedItems();
    button->setEnabled(!list.empty() & m_refreshEnabled & state);
    button = m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button");
    button->setEnabled(state & !m_refreshEnabled & !list.empty());
}

void Engine::Kernel::handleAuthentication(PowerStateValues::POWER_VALUES state)
{
    QTreeWidgetItem* item = m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0];
    std::string ip = item->text(0).toStdString();
    AuthenticationDialog dialog(ip);
    if (dialog.exec()) {
        std::string username = dialog.getUsername();
        std::string passwd = dialog.getPasswd();
        if (username == "" || passwd == "")
            return;

        GnomeKeyringResult res =
                gnome_keyring_store_password_sync(
                    GNOME_KEYRING_NETWORK_PASSWORD,
                    OPENLMI_KEYRING_DEFAULT,
                    ip.c_str(),
                    passwd.c_str(),
                    "user", username.c_str(),
                    "server", ip.c_str(),
                    NULL
                    );

        if (res != GNOME_KEYRING_RESULT_OK) {
            Logger::getInstance()->error("Cannot store password!");
            return;
        }

        boost::thread(boost::bind(&Engine::Kernel::getConnection, this, state));
    } else {
        handleProgressState(1);
        m_main_window.getStatusBar()->clearMessage();
    }
}

void Engine::Kernel::handleConnecting(CIMClient *client, PowerStateValues::POWER_VALUES state)
{
    if (client == NULL) {
        handleProgressState(1);
        return;
    }

    if (state == PowerStateValues::NoPowerSetting) {
        QTabWidget* tab = m_main_window.getProviderWidget()->getTabWidget();
        IPlugin *plugin = (IPlugin*) tab->currentWidget();

        if (plugin != NULL)
            plugin->refresh(client);
    } else {
        setPowerState(client, state);
    }
}

void Engine::Kernel::handleError(std::string message)
{
    m_main_window.getStatusBar()->clearMessage();
    Logger::getInstance()->error(message);
}

void Engine::Kernel::handleInstructionText(std::string text)
{
    m_code_dialog.setText(text, false);
}

void Engine::Kernel::handleProgressState(int state)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();    
    if (state == 100) {
        tab->setEnabled(true);
        plugin->setPluginEnabled(true);
        setButtonsEnabled(true);
        m_main_window.getStatusBar()->clearMessage();
        m_bar->hide();
    } else if (state == 0) {
        plugin->setPluginEnabled(false);
        m_bar->setMaximum(0);
        m_bar->show();
    } else {
        tab->setEnabled(true);
        plugin->setPluginEnabled(false);
        enableSpecialButtons(true);
        m_bar->hide();
    }
}

void Engine::Kernel::rebootPc()
{
    QList<QTreeWidgetItem *> items;
    if (!(items = m_main_window.getPcTreeWidget()->getTree()->selectedItems()).isEmpty())
        Logger::getInstance()->info("Rebooting system: "
                                    + m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]->text(0).toStdString()
                                    );

    handleProgressState(0);
    boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerCycleOffSoft));
}

void Engine::Kernel::refresh()
{    
    if (!m_refreshEnabled || m_main_window.getPcTreeWidget()->getTree()->selectedItems().empty())
        return;

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();

    if (plugin == NULL)
        return;

    tab->setEnabled(false);
    setButtonsEnabled(false);
    handleProgressState(0);
    boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::NoPowerSetting));
}

void Engine::Kernel::resetKeyring()
{
    GnomeKeyringResult res = gnome_keyring_delete_sync(OPENLMI_KEYRING_DEFAULT);
    if (res != GNOME_KEYRING_RESULT_OK) {
        Logger::getInstance()->error("Cannot reset keyring");
        return;
    }
    createKeyring();
}

void Engine::Kernel::setActivePlugin(int index)
{
    int i = 0;
    for (plugin_map::iterator it = m_loaded_plugins.begin(); it != m_loaded_plugins.end(); it++) {        
        if (i == index) {
            (*it).second->setActive(true);
            m_code_dialog.setText((*it).second->getInstructionText(), false);
            setButtonsEnabled((*it).second->isRefreshed(), false);
            QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");
            if (button != NULL)
                button->setChecked((*it).second->isFilterShown());
        } else
            (*it).second->setActive(false);
        i++;
    }
}

void Engine::Kernel::setEditState(bool state)
{
    PCTreeWidget* tree_widget = m_main_window.getPcTreeWidget();
    m_refreshEnabled = !state;
    m_main_window.getProviderWidget()->setEnabled(!state);
    setButtonsEnabled(!state);
    bool empty = m_main_window.getPcTreeWidget()->getTree()->selectedItems().empty();
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("delete_passwd_button"))->setEnabled(state & !empty);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("add_button"))->setEnabled(state);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("remove_button"))->setEnabled(state);
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("discover_button"))->setEnabled(state);    
    ((QPushButton*) m_main_window.getToolbar()->findChild<QPushButton*>("edit_button"))->setIcon(
                QIcon(!state ? "../../icons/changes-prevent.png" : "../../icons/changes-allow.png"));

    tree_widget->setEditState(state);
}

void Engine::Kernel::setPluginNoChanges(IPlugin *plugin)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setTabText(getIndexOfTab(plugin->getLabel()), plugin->getLabel().c_str());
}

void Engine::Kernel::setPluginUnsavedChanges(IPlugin *plugin)
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    tab->setTabText(getIndexOfTab(plugin->getLabel()), std::string("* " + plugin->getLabel()).c_str());
}

void Engine::Kernel::showCodeDialog()
{
    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();
    m_code_dialog.setText(plugin->getInstructionText(), false);
    m_code_dialog.show();
}

void Engine::Kernel::showFilter()
{
    QPushButton *button = m_main_window.getToolbar()->findChild<QPushButton*>("filter_button");

    QTabWidget *tab = m_main_window.getProviderWidget()->getTabWidget();
    IPlugin *plugin = (IPlugin*) tab->currentWidget();

    if (plugin == NULL) {
        button->setChecked(false);
        return;
    }

    plugin->showFilter(button->isChecked());
}

void Engine::Kernel::shutdownPc()
{
    QList<QTreeWidgetItem *> items;
    if (!(items = m_main_window.getPcTreeWidget()->getTree()->selectedItems()).isEmpty())
        Logger::getInstance()->info("Shutting down system: "
                                    + m_main_window.getPcTreeWidget()->getTree()->selectedItems()[0]->text(0).toStdString()
                                    );

    handleProgressState(0);
    boost::thread(boost::bind(&Engine::Kernel::getConnection, this, PowerStateValues::PowerOffSoftGraceful));
}
