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

#include "detailsdialog.h"
#include "softwareprovider.h"
#include "lmiwbem_value.h"
#include "ui_softwareprovider.h"

#include <boost/thread.hpp>
#include <sstream>

void SoftwareProviderPlugin::fetchPackageInfo(Pegasus::CIMInstance instance)
{
    Pegasus::CIMInstance package;

    try {
        Pegasus::Uint32 prop_ind = instance.findProperty("InstalledSoftware");
        Pegasus::CIMValue value = instance.getProperty(prop_ind).getValue();

        Pegasus::CIMObjectPath path;
        if (!value.isNull())
            value.get(path);

        package = m_client->getInstance(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    path
                    );
    } catch (Pegasus::Exception &ex) {
        refreshProgress(1);
        return;
    }

    emit havePackageDetails(package);
}

SoftwareProviderPlugin::SoftwareProviderPlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::SoftwareProviderPlugin)
{
    m_ui->setupUi(this);
    showFilter(false);
    setPluginEnabled(false);
    hideRepoButtons();

    connect(
        m_ui->installed,
        SIGNAL(itemDoubleClicked(QListWidgetItem*)),
        this,
        SLOT(getPackageDetail(QListWidgetItem*)));
    connect(m_ui->repos,
        SIGNAL(itemDoubleClicked(QListWidgetItem*)),
        this,
        SLOT(showRepoDetail(QListWidgetItem*)));
    qRegisterMetaType<Pegasus::CIMInstance>("Pegasus::CIMInstance");
    connect(
        this,
        SIGNAL(havePackageDetails(Pegasus::CIMInstance)),
        this,
        SLOT(showPackageDetail(Pegasus::CIMInstance)));
    connect(
        m_ui->installed,
        SIGNAL(getFocus()),
        this,
        SLOT(hideRepoButtons()));
    connect(
        m_ui->repos,
        SIGNAL(getFocus()),
        this,
        SLOT(hidePackageButtons()));
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
    Pegasus::Array<Pegasus::CIMInstance> installed;
    Pegasus::Array<Pegasus::CIMInstance> repos;
    std::string filter = m_ui->filter_line->text().toStdString();

    try {        
        // installed packages
        installed = m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_InstalledSoftwareIdentity"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
                );

        std::vector<Pegasus::CIMInstance> *vector = new std::vector<Pegasus::CIMInstance>();
        data->push_back(vector);

        for (unsigned int i = 0; i < installed.size(); i++) {
            Pegasus::Uint32 prop_ind = installed[i].findProperty("InstalledSoftware");
            Pegasus::CIMProperty property = installed[i].getProperty(prop_ind);

            std::string name;
            if (!property.getValue().isNull())
                name = property.getValue().toString().getCString();

            if (filter.empty())
                vector->push_back(installed[i]);
            else if (name.find(filter) != std::string::npos)
                vector->push_back(installed[i]);
        }

        // repos
        repos = m_client->enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_SoftwareIdentityResource"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
                );

        vector = new std::vector<Pegasus::CIMInstance>();
        data->push_back(vector);
        for (unsigned int i = 0; i < repos.size(); i++) {
            vector->push_back(repos[i]);
        }
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
        std::vector<Pegasus::CIMInstance> *vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[0]);

        for (unsigned int i = 0; i < vector->size(); i++) {
            Pegasus::Uint32 prop_ind = (*vector)[i].findProperty("InstalledSoftware");
            Pegasus::CIMProperty property = (*vector)[i].getProperty(prop_ind);

            std::string name;
            if (!property.getValue().isNull())
                name = property.getValue().toString().getCString();

            int ch = name.rfind(":", name.rfind(":") - 1);
            name = name.substr(ch + 1, name.length() - ch - 2);

            m_ui->installed->addItem(new QListWidgetItem(name.c_str()));
        }
        m_installed = *vector;

        vector = ((std::vector<Pegasus::CIMInstance> *) (*data)[1]);
        for (unsigned int i = 0; i < vector->size(); i++) {
            m_ui->repos->addItem(new QListWidgetItem(getPropertyOfInstance((*vector)[i], "Caption").c_str()));
        }
        m_repos = *vector;
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    m_changes_enabled = true;
}

void SoftwareProviderPlugin::getPackageDetail(QListWidgetItem *item)
{
    emit refreshProgress(0);

    if (item != NULL) {
        std::string id = item->text().toStdString();

        for (unsigned int i = 0; i < m_installed.size(); i++) {
            Pegasus::Uint32 prop_ind = m_installed[i].findProperty("InstalledSoftware");
            Pegasus::CIMProperty property = m_installed[i].getProperty(prop_ind);

            std::string name;
            if (!property.getValue().isNull())
                name = property.getValue().toString().getCString();

            int ch = name.rfind(":", name.rfind(":") - 1);
            name = name.substr(ch + 1, name.length() - ch - 2);

            if (name != id)
                continue;

            boost::thread(boost::bind(&SoftwareProviderPlugin::fetchPackageInfo, this, m_installed[i]));
            break;
        }
    }
}

void SoftwareProviderPlugin::showPackageDetail(Pegasus::CIMInstance item)
{
    emit refreshProgress(100);

    DetailsDialog dialog("Package details", this);
    dialog.setValues(item, true);
    dialog.exec();
}

void SoftwareProviderPlugin::showRepoDetail(QListWidgetItem *item)
{
    Pegasus::CIMInstance repo;
    for (unsigned int i = 0; i < m_repos.size(); i++) {
        if (item->text().toStdString() == getPropertyOfInstance(m_repos[i], "Caption")) {
            repo = m_repos[i];
            break;
        }
    }

    DetailsDialog dialog("Repo details", this);
    dialog.setValues(repo, true);
    dialog.exec();
}

void SoftwareProviderPlugin::hidePackageButtons()
{
    m_ui->enable_repo_button->show();
    m_ui->disable_repo_button->show();
    m_ui->verify_package_button->hide();
    m_ui->install_button->hide();
    m_ui->uninstall_button->hide();
    m_ui->update_button->hide();
}

void SoftwareProviderPlugin::hideRepoButtons()
{
    m_ui->enable_repo_button->hide();
    m_ui->disable_repo_button->hide();
    m_ui->verify_package_button->show();
    m_ui->install_button->show();
    m_ui->uninstall_button->show();
    m_ui->update_button->show();
}

Q_EXPORT_PLUGIN2(softwareProvider, SoftwareProviderPlugin)
