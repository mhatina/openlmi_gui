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

#ifndef NETWORKPAGEWIDGET_H
#define NETWORKPAGEWIDGET_H

#include "plugin.h"

#include <Pegasus/Common/CIMInstance.h>
#include <QWidget>

const Engine::header_item items[] = {
    {"IP Address", "IP address is a numerical label assigned to each device participating in a computer network that uses the Internet Protocol for communication."},
    {"Subnet Mask", "The mask for the IPv4 address of this ProtocolEndpoint, if one is defined."},
    {"IPv6 Subnet Prefix Length", "IPv6 Subnet Prefix Length is used to identify the prefix length of the IPv6Address property that is used to specify a subnet"},
    {"Protocol IF Type", "This property explicitly defines support for different versions of the IP protocol."},
    {"Address Origin", "AddressOrigin identifies the method by which the IP Address, Subnet Mask, and Gateway were assigned to the IPProtocolEndpoint."},
    {"Access Context", "The AccessContext property identifies the role this RemoteServiceAccessPoint is playing in the hosting system."},
    {"Access Info", "Access or addressing information or a combination of this information for a remote connection. This information can be a host name, network address, or similar information."}
};

namespace Ui
{
class NetworkPageWidget;
}

class NetworkPageWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::NetworkPageWidget *m_ui;

public:
    explicit NetworkPageWidget(QWidget *parent = 0);
    ~NetworkPageWidget();

    std::string getTitle();
    void setEthernetPort(Pegasus::CIMInstance instance);
    void setEthernetStatistics(Pegasus::CIMInstance instance);
    void setIPNetworkConnection(Pegasus::CIMInstance instance);
    void setIPProtocolEndpoint(Pegasus::CIMInstance instance);
    void setLanEndPoint(Pegasus::CIMInstance instance);
    void setNetworkRemoteService(Pegasus::CIMInstance instance);
    void setDNSSettingData(Pegasus::CIMInstance instance);
};

#endif // NETWORKPAGEWIDGET_H
