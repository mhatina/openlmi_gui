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

#ifndef NETWORK_H
#define NETWORK_H

#include "lmi_string.h"
#include "networkpagewidget.h"
#include "plugin.h"

#include <QtPlugin>

namespace Ui
{
class NetworkPlugin;
}

class NetworkPlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    bool m_changes_enabled;
    Ui::NetworkPlugin *m_ui;

    NetworkPageWidget *findWidget(String text);

public:
    explicit NetworkPlugin();
    ~NetworkPlugin();
    virtual String getInstructionText();
    virtual String getLabel();
    virtual String getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);
};

#endif // NETWORK_H
