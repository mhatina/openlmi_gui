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

#include "softwareprovider.h"
#include "lmiwbem_value.h"
#include "ui_softwareprovider.h"

#include <sstream>

SoftwareProviderPlugin::SoftwareProviderPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::SoftwareProviderPlugin)
{
    m_ui->setupUi(this);
    showFilter(false);
    setPluginEnabled(false);
}

SoftwareProviderPlugin::~SoftwareProviderPlugin()
{
    delete m_ui;
}

std::string SoftwareProviderPlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string SoftwareProviderPlugin::getLabel()
{
    return "Sof&tware";
}

void SoftwareProviderPlugin::getData(std::vector<void *> *data)
{    
    try {    
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, std::string(ex.getMessage().getCString()));
        return;
    }

    emit doneFetchingData(data);
}

void SoftwareProviderPlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    try {
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    m_changes_enabled = true;
}

Q_EXPORT_PLUGIN2(softwareProvider, SoftwareProviderPlugin)