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

#include "cimvalue.h"
#include "networkpagewidget.h"
#include "ui_networkpagewidget.h"

NetworkPageWidget::NetworkPageWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::NetworkPageWidget)
{
    m_ui->setupUi(this);

    m_ui->ip_address_details_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_ui->ip_address_details_table->setMaximumHeight(25);

    int cnt = sizeof(items) / sizeof(items[0]);
    m_ui->ip_address_details_table->setColumnCount(cnt);
    for (int i = 0; i < cnt; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(items[i].name);
        item->setToolTip(items[i].tooltip);
        m_ui->ip_address_details_table->setHorizontalHeaderItem(
            i,
            item
        );
    }
}

NetworkPageWidget::~NetworkPageWidget()
{
    delete m_ui;
}

std::string NetworkPageWidget::getTitle()
{
    return m_ui->network_box->title().toStdString();
}

void NetworkPageWidget::setEthernetPort(Pegasus::CIMInstance instance)
{
    m_ui->network_box->setTitle(CIMValue::get_property_value(instance, "DeviceID").c_str());
    std::string max_speed = CIMValue::get_property_value(instance, "MaxSpeed");
    if (!max_speed.empty()) {
        m_ui->max_speed->show();
        m_ui->max_speed_label->show();

        max_speed = CIMValue::convert_values(max_speed, "b/s");
        m_ui->max_speed->setText(max_speed.c_str());
    } else {
        m_ui->max_speed->hide();
        m_ui->max_speed_label->hide();
    }

    std::string address = CIMValue::get_property_value(instance, "PermanentAddress");
    m_ui->mac_address->setText(address.empty() ? "N/A" : address.c_str());
}

void NetworkPageWidget::setEthernetStatistics(Pegasus::CIMInstance instance)
{
    m_ui->sent->setText(CIMValue::convert_values(CIMValue::get_property_value(instance, "BytesTransmitted"), "B").c_str());
    m_ui->received->setText(CIMValue::convert_values(CIMValue::get_property_value(instance, "BytesReceived"), "B").c_str());
}

void NetworkPageWidget::setIPNetworkConnection(Pegasus::CIMInstance instance)
{
    m_ui->status->setText(CIMValue::get_property_value(instance, "OperatingStatus").c_str());
}

void NetworkPageWidget::setIPProtocolEndpoint(Pegasus::CIMInstance instance)
{
    QTableWidgetItem *item;
    int i = 0;
    while (1) {
        item = m_ui->ip_address_details_table->item(i, 0);
        if (item == NULL) {
            break;
        }
        i++;
    }

    if (i >= m_ui->ip_address_details_table->rowCount()) {
        int row_cnt = m_ui->ip_address_details_table->rowCount();
        m_ui->ip_address_details_table->insertRow(row_cnt);
    }

    bool v4 = false;
    std::string version;
    item = new QTableWidgetItem((version = CIMValue::get_property_value(instance,
            "ProtocolIFType")).c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        3,
        item);
    v4 = version == "IPv4";

    item = new QTableWidgetItem(CIMValue::get_property_value(instance, v4 ? "IPv4Address" : "IPv6Address").c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        0,
        item);

    item = new QTableWidgetItem(CIMValue::get_property_value(instance,
                                v4 ? "SubnetMask" : "IPv6SubnetPrefixLength").c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        v4 ? 1 : 2,
        item);

    item = new QTableWidgetItem(CIMValue::get_property_value(instance, "AddressOrigin").c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        4,
        item);

    int max_height = m_ui->ip_address_details_table->maximumHeight() + 30;
    m_ui->ip_address_details_table->setMaximumHeight(max_height);
}

void NetworkPageWidget::setLanEndPoint(Pegasus::CIMInstance instance)
{
    if (m_ui->mac_address->text() == "N/A") {
        m_ui->mac_address->setText(CIMValue::get_property_value(instance, "MACAddress").c_str());
    }
}

void NetworkPageWidget::setNetworkRemoteService(Pegasus::CIMInstance instance)
{
    QTableWidgetItem *item;
    int i = 0;
    while (1) {
        item = m_ui->ip_address_details_table->item(i, 5);
        if (item == NULL) {
            break;
        }
        i++;
    }

    if (i >= m_ui->ip_address_details_table->rowCount()) {
        int row_cnt = m_ui->ip_address_details_table->rowCount();
        m_ui->ip_address_details_table->insertRow(row_cnt);
    }

    item = new QTableWidgetItem(CIMValue::get_property_value(instance, "AccessContext").c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        5,
        item);

    item = new QTableWidgetItem(CIMValue::get_property_value(instance, "AccessInfo").c_str());
    item->setToolTip(item->text());
    m_ui->ip_address_details_table->setItem(
        i,
        6,
        item);
}
