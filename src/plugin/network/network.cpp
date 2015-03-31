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

#include "network.h"
#include "networkpagewidget.h"
#include "cimvalue.h"
#include "ui_network.h"

#include <sstream>
#include <QTableWidgetItem>

QTableWidgetItem *createItem(std::string text)
{
    QTableWidgetItem *item = new QTableWidgetItem(text.c_str());
    item->setToolTip(text.c_str());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    return item;
}

NetworkPageWidget *NetworkPlugin::findWidget(std::string text)
{
    int cnt = m_ui->network_widgets->count();
    for (int i = 0; i < cnt; i++) {
        NetworkPageWidget *w = (NetworkPageWidget *) m_ui->network_widgets->widget(i);
        if (text.find(w->getTitle()) != std::string::npos) {
            return w;
        }
    }

    return NULL;
}

NetworkPlugin::NetworkPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::NetworkPlugin)
{
    m_ui->setupUi(this);
    m_ui->filter_box->hide();
    setPluginEnabled(false);

    connect(
        m_ui->scroll_bar,
        SIGNAL(valueChanged(int)),
        m_ui->network_widgets,
        SLOT(setCurrentIndex(int)));
}

NetworkPlugin::~NetworkPlugin()
{
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
}

std::string NetworkPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string NetworkPlugin::getLabel()
{
    return "Network";
}

std::string NetworkPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": " << m_ui->scroll_bar->maximum() + 1 << " interfaces shown";
    return ss.str();
}

void NetworkPlugin::getData(std::vector<void *> *data)
{
    try {
        Pegasus::Array<Pegasus::CIMInstance> device =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_EthernetPort"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );
        int cnt = device.size();
        for (int i = 0; i < cnt; i++) {
            data->push_back(new Pegasus::CIMInstance(device[i]));
        }

        Pegasus::Array<Pegasus::CIMInstance> stats =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_EthernetPortStatistics"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );
        cnt = stats.size();
        for (int i = 0; i < cnt; i++) {
            data->push_back(new Pegasus::CIMInstance(stats[i]));
        }

        Pegasus::Array<Pegasus::CIMInstance> network =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_IPNetworkConnection"),
                true,       // deep inheritance
                false,      // local only
                true,       // include qualifiers
                false       // include class origin
            );

        cnt = network.size();
        for (int i = 0; i < cnt; i++) {
            Pegasus::Array<Pegasus::CIMObject> net_assoc =
                associators(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    network[i].getPath(),
                    Pegasus::CIMName(),
                    Pegasus::CIMName(),
                    Pegasus::String::EMPTY,
                    Pegasus::String::EMPTY,
                    true
                );

            data->push_back(new Pegasus::CIMInstance(network[i]));
            int cnt_assoc = net_assoc.size();
            for (int j = 0; j < cnt_assoc; j++) {
                data->push_back(new Pegasus::CIMInstance(net_assoc[j]));
            }
        }
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    emit doneFetchingData(data);
}

void NetworkPlugin::clear()
{
    m_changes_enabled = false;

    int cnt = m_ui->network_widgets->count();
    for (int i = cnt - 1; i >= 0; i--) {
        QWidget *w = m_ui->network_widgets->widget(i);
        m_ui->network_widgets->removeWidget(w);
        if (w != NULL) {
                delete w;
                w = NULL;
            }
    }

    m_ui->scroll_bar->setMaximum(0);

    m_changes_enabled = true;
}

void NetworkPlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    int page_count = -1;

    try {
        int cnt = data->size();
        for (int i = 0; i < cnt; i++) {
            Pegasus::CIMInstance instance = (*(Pegasus::CIMInstance *) (*data)[i]);

            if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_EthernetPort") {
                NetworkPageWidget *network_widget = new NetworkPageWidget();
                m_ui->network_widgets->addWidget(network_widget);
                network_widget->setEthernetPort(instance);

                page_count++;
                if (m_ui->scroll_bar->maximum() < page_count) {
                    m_ui->scroll_bar->setMaximum(m_ui->scroll_bar->maximum() + 1);
                }
            } else if (CIMValue::get_property_value(instance, "InstanceID").find("LMI_EthernetPortStatistics") != std::string::npos) {
                NetworkPageWidget *w = findWidget(CIMValue::get_property_value(instance, "ElementName"));
                w->setEthernetStatistics(instance);
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_IPNetworkConnection") {
                NetworkPageWidget *w = findWidget(CIMValue::get_property_value(instance, "ElementName"));
                w->setIPNetworkConnection(instance);
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_LANEndpoint") {
                NetworkPageWidget *w = findWidget(CIMValue::get_property_value(instance, "ElementName"));
                w->setLanEndPoint(instance);
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_IPProtocolEndpoint") {
                NetworkPageWidget *w = findWidget(CIMValue::get_property_value(instance, "Name"));
                w->setIPProtocolEndpoint(instance);
            } else if (CIMValue::get_property_value(instance, "CreationClassName") == "LMI_NetworkRemoteServiceAccessPoint") {
                NetworkPageWidget *w = findWidget(CIMValue::get_property_value(instance, "Name"));
                w->setNetworkRemoteService(instance);
            }
        }

    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    m_changes_enabled = true;
}

Q_EXPORT_PLUGIN2(network, NetworkPlugin)
