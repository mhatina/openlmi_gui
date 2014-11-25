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

#include "actionbox.h"
#include "dialogs/detailsdialog.h"
#include "instructions/disableserviceinstruction.h"
#include "instructions/enableserviceinstruction.h"
#include "instructions/getinstruction.h"
#include "instructions/reloadserviceinstruction.h"
#include "instructions/restartserviceinstruction.h"
#include "instructions/startserviceinstruction.h"
#include "instructions/stopserviceinstruction.h"
#include "cimvalue.h"
#include "service.h"
#include "ui_service.h"

#include <QMenu>
#include <sstream>
#include <vector>

void ServicePlugin::initContextMenu()
{
    Logger::getInstance()->debug("ServicePlugin::initContextMenu()");
    m_context_menu = new QMenu(this);
    m_context_menu->setObjectName("service_context_menu");
    setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *action;

    int cnt = sizeof(action_list) / sizeof(action_list[0]);
    for (int i = 0; i < cnt; i++) {
        action = m_context_menu->addAction(action_list[i]);
        std::string object_name = action_list[i];
        object_name += "_action";
        action->setObjectName(object_name.c_str());
    }
    connect(
        m_context_menu,
        SIGNAL(triggered(QAction*)),
        this,
        SLOT(actionHandle(QAction*)));

    m_context_menu->addSeparator();

    action = m_context_menu->addAction("Show details");
    action->setObjectName("show_details_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showDetails()));
}

ServicePlugin::ServicePlugin() :
    IPlugin(),
    m_ui(new Ui::ServicePlugin)
{
    m_ui->setupUi(this);
    m_services_table = m_ui->services_table;
    int prop_cnt = sizeof(serviceHeaderItem) / sizeof(serviceHeaderItem[0]);
    m_services_table->setColumnCount(prop_cnt);
    m_services_table->setShowGrid(false);
    for (int i = 0; i < prop_cnt; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(serviceHeaderItem[i].name);
        item->setToolTip(serviceHeaderItem[i].tooltip);
        m_services_table->setHorizontalHeaderItem(
            i,
            item
        );
    }

    initContextMenu();

    m_services_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    connect(
        m_ui->services_table,
        SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
        this,
        SLOT(showDetails()));
    connect(
        this,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showContextMenu(QPoint)));
    m_ui->filter_box->hide();
    setPluginEnabled(false);
}

ServicePlugin::~ServicePlugin()
{
    delete m_ui;
}

std::string ServicePlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string ServicePlugin::getLabel()
{
    return "Service";
}

std::string ServicePlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": " << m_service_instances.size() << " service(s) shown";
    return ss.str();
}

void ServicePlugin::getData(std::vector<void *> *data)
{
    Pegasus::Array<Pegasus::CIMInstance> services;
    std::string filter = m_ui->filter_line->text().toStdString();

    try {
        services = enumerateInstances(
                       Pegasus::CIMNamespaceName("root/cimv2"),
                       Pegasus::CIMName("LMI_Service"),
                       true,       // deep inheritance
                       false,      // local only
                       true,       // include qualifiers
                       false       // include class origin
                   );
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    unsigned int cnt = services.size();
    for (unsigned int i = 0; i < cnt; i++) {
        Pegasus::CIMInstance instance;
        if (!filter.empty()) {
            instance = services[i];
            if (CIMValue::get_property_value(instance,
                                             "Name").find(filter) == std::string::npos) {
                continue;
            }
        } else {
            instance = services[i];
        }

        data->push_back(new Pegasus::CIMInstance(instance));
        m_service_instances.push_back(instance);
    }

    emit doneFetchingData(data);
}

void ServicePlugin::clear()
{
    m_changes_enabled = false;
    m_services_table->setRowCount(0);
    m_changes_enabled = true;
}

void ServicePlugin::fillTab(std::vector<void *> *data)
{
    clear();
    m_changes_enabled = false;

    try {
        Pegasus::Uint32 services_cnt = data->size();
        for (unsigned int i = 0; i < services_cnt; i++) {
            int row_count = m_services_table->rowCount();
            m_services_table->insertRow(row_count);

            int prop_cnt = sizeof(serviceProperties) / sizeof(serviceProperties[0]);
            std::string serv_name;
            for (int j = 0; j < prop_cnt; j++) {
                Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[i]);
                Pegasus::Uint32 propIndex = instance->findProperty(Pegasus::CIMName(
                                                serviceProperties[j].property));
                if (propIndex == Pegasus::PEG_NOT_FOUND) {
                    Logger::getInstance()->error("property " + std::string(
                                                     serviceProperties[j].property) + " not found");
                    continue;
                }

                Pegasus::CIMProperty property = instance->getProperty(propIndex);
                std::string str_value = CIMValue::get_property_value(*instance, CIMValue::to_std_string(property.getName().getString()));

                if (property.getName().equal(Pegasus::CIMName("Name"))) {
                    serv_name = str_value;
                }

                QTableWidgetItem *item =
                    new QTableWidgetItem(str_value.c_str());
                item->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                item->setToolTip(str_value.c_str());
                m_services_table->setItem(
                    row_count,
                    j,
                    item
                );

                if (!serviceProperties[j].enabled) {
                    m_services_table->item(row_count, j)->setFlags(item_flags);
                }
            }

            ActionBox *box = new ActionBox(serv_name);
            connect(
                box,
                SIGNAL(performAction(std::string, e_action)),
                this,
                SLOT(actionHandle(std::string, e_action)));
            m_services_table->setCellWidget(
                row_count,
                prop_cnt,
                box);
        }

        m_services_table->sortByColumn(0, Qt::AscendingOrder);
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
    }

    for (unsigned int i = 0; i < data->size(); i++) {
        delete ((Pegasus::CIMInstance *)(*data)[i]);
    }

    m_changes_enabled = true;
}

void ServicePlugin::actionHandle(QAction *action)
{
    QList<QTableWidgetItem *> list = m_ui->services_table->selectedItems();
    if (list.empty())
        return;

    int row = list[0]->row();

    ActionBox *box = (ActionBox *) m_ui->services_table->cellWidget(row, 4);
    box->changeAction(action->text().toStdString());
}

void ServicePlugin::actionHandle(std::string name, e_action action)
{
    addInstruction(new GetInstruction(name));

    switch (action) {
    case RELOAD:
        addInstruction(new ReloadServiceInstruction(m_client, name));
        break;
    case RESTART:
        addInstruction(new RestartServiceInstruction(m_client, name));
        break;
    case START:
        addInstruction(new StartServiceInstruction(m_client, name));
        break;
    case STOP:
        addInstruction(new StopServiceInstruction(m_client, name));
        break;
    case ENABLE:
        addInstruction(new EnableServiceInstruction(m_client, name));
        break;
    case DISABLE:
        addInstruction(new DisableServiceInstruction(m_client, name));
        break;
    default:
        return;
    }
}

void ServicePlugin::showContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("ServicePlugin::showContextMenu(QPoint pos)");
    QPoint globalPos = mapToGlobal(pos);
    m_context_menu->popup(globalPos);
}

void ServicePlugin::showDetails()
{
    Pegasus::CIMInstance service;
    QList<QTableWidgetItem *> list = m_ui->services_table->selectedItems();
    if (list.empty())
        return;

    std::string name_expected = list[0]->text().toStdString();
    int cnt = m_service_instances.size();
    for (int i = 0; i < cnt; i++) {
        if (name_expected == CIMValue::get_property_value(m_service_instances[i],
                "Name")) {
            service = m_service_instances[i];
        }
    }

    std::map<std::string, std::string> values;
    cnt = service.getPropertyCount();
    for (int i = 0; i < cnt; i++) {
        std::string object_name = CIMValue::to_std_string(service.getProperty(i).getName().getString());
        std::string str_value = CIMValue::get_property_value(service, object_name);
        values[object_name] = str_value;
    }

    DetailsDialog dialog("Service details", this);
    dialog.setValues(values, true);
    dialog.hideCancelButton();
    dialog.exec();
}

Q_EXPORT_PLUGIN2(service, ServicePlugin)
