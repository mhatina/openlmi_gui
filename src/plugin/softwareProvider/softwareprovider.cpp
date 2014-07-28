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
#include "instructions/disablerepoinstruction.h"
#include "instructions/enablerepoinstruction.h"
#include "instructions/installpackageinstruction.h"
#include "instructions/uninstallpackageinstruction.h"
#include "instructions/updatepackageinstruction.h"
#include "instructions/verifypackageinstruction.h"
#include "lmiwbem_value.h"
#include "softwareprovider.h"
#include "ui_softwareprovider.h"

#include <algorithm>
#include <boost/thread.hpp>
#include <sstream>

Pegasus::CIMInstance SoftwareProviderPlugin::findInstalledPackage(std::string package_name)
{
    Pegasus::CIMInstance instance;
    for (unsigned int i = 0; i < m_installed.size(); i++) {
        Pegasus::Uint32 prop_ind = m_installed[i].findProperty("InstalledSoftware");
        std::string prop = (std::string) m_installed[i].getProperty(prop_ind).getValue().toString().getCString();
        if (prop.find(package_name) != std::string::npos) {
            instance = m_installed[i];
            break;
        }
    }

    return instance;
}

Pegasus::CIMInstance SoftwareProviderPlugin::findRepo(std::string repo_name)
{
    Pegasus::CIMInstance instance;
    for (unsigned int i = 0; i < m_repos.size(); i++) {
        Pegasus::Uint32 prop_ind = m_repos[i].findProperty("Caption");
        std::string prop = (std::string) m_repos[i].getProperty(prop_ind).getValue().toString().getCString();
        if (prop.find(repo_name) != std::string::npos) {
            instance = m_repos[i];
            break;
        }
    }

    return instance;
}

std::string SoftwareProviderPlugin::getPackageName(Pegasus::CIMInstance package)
{
    Pegasus::Uint32 prop_ind = package.findProperty("InstalledSoftware");
    Pegasus::CIMProperty property = package.getProperty(prop_ind);

    std::string name;
    if (property.getValue().isNull())
        return "";

    name = property.getValue().toString().getCString();

    int ch = name.rfind(":", name.rfind(":") - 1);
    name = name.substr(ch + 1, name.length() - ch - 2);
    return name;
}

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
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(disablePackageButtons()));
    connect(
        m_ui->repos,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(disableRepoButtons()));
    connect(
        m_ui->disable_repo_button,
        SIGNAL(clicked()),
        this,
        SLOT(disableRepo()));
    connect(
        m_ui->enable_repo_button,
        SIGNAL(clicked()),
        this,
        SLOT(enableRepo()));
    connect(
        m_ui->install_button,
        SIGNAL(clicked()),
        this,
        SLOT(installPackage()));
    connect(
        m_ui->uninstall_button,
        SIGNAL(clicked()),
        this,
        SLOT(uninstallPackage()));
    connect(
        m_ui->verify_package_button,
        SIGNAL(clicked()),
        this,
        SLOT(verifyPackage()));
    connect(
        m_ui->update_button,
        SIGNAL(clicked()),
        this,
        SLOT(updatePackage()));
    connect(
        m_ui->filter_line,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(updateList()));
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
    return "Software";
}

std::string SoftwareProviderPlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << m_installed.size() << " installed package(s), "
       << m_repos.size() << " repository(ies) shown";
    return ss.str();
}

void SoftwareProviderPlugin::getData(std::vector<void *> *data)
{    
    Pegasus::Array<Pegasus::CIMInstance> installed;
    Pegasus::Array<Pegasus::CIMInstance> repos;
    Pegasus::Array<Pegasus::CIMInstance> verify;
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

        if (!m_verify.empty()) {
            verify = m_client->enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_SoftwareVerificationJob"),
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

    m_ui->repos->clear();
    m_ui->installed->clear();

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
            QListWidgetItem *item = new QListWidgetItem(getPropertyOfInstance((*vector)[i], "Caption").c_str());
            item->setIcon(QIcon(getPropertyOfInstance((*vector)[i], "EnabledState") == "2" ? ":/enabled.png" : ":/disabled.png"));
            m_ui->repos->addItem(item);
        }
        m_repos = *vector;
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
        return;
    }

    for (unsigned int i = 0; i < data->size(); i++)
        delete ((std::vector<Pegasus::CIMInstance> *) (*data)[i]);

    disableRepoButtons();
    disablePackageButtons();

    m_changes_enabled = true;
}

void SoftwareProviderPlugin::disablePackageButtons()
{
    bool empty = m_ui->installed->selectedItems().empty();

    m_ui->verify_package_button->setEnabled(!empty);
    m_ui->uninstall_button->setEnabled(!empty);
    m_ui->update_button->setEnabled(!empty);
}

void SoftwareProviderPlugin::disableRepo()
{
    if (m_ui->repos->selectedItems().empty())
        return;

    for (int i = 0; i < m_ui->repos->selectedItems().size(); i++) {
        QListWidgetItem *item = m_ui->repos->selectedItems()[i];

        item->setIcon(QIcon(":/disabled.png"));
        addInstruction(
                new DisableRepoInstruction(
                        m_client,
                        findRepo(item->text().toStdString())
                        )
                    );
    }
}

void SoftwareProviderPlugin::disableRepoButtons()
{
    bool empty = m_ui->repos->selectedItems().empty();

    m_ui->enable_repo_button->setEnabled(!empty);
    m_ui->disable_repo_button->setEnabled(!empty);
}

void SoftwareProviderPlugin::enableRepo()
{
    if (m_ui->repos->selectedItems().empty())
        return;

    for (int i = 0; i < m_ui->repos->selectedItems().size(); i++) {
        QListWidgetItem *item = m_ui->repos->selectedItems()[i];

        item->setIcon(QIcon(":/enabled.png"));
        addInstruction(
                new EnableRepoInstruction(
                        m_client,
                        findRepo(item->text().toStdString())
                        )
                    );
    }
}

void SoftwareProviderPlugin::getPackageDetail(QListWidgetItem *item)
{
    emit refreshProgress(0);

    if (item != NULL) {
        std::string id = item->text().toStdString();

        for (unsigned int i = 0; i < m_installed.size(); i++) {            
            if (getPackageName(m_installed[i]) != id)
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

void SoftwareProviderPlugin::installPackage()
{
    Logger::getInstance()->error("Not yet supported!");
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

void SoftwareProviderPlugin::uninstallPackage()
{
    QList<QListWidgetItem*> list = m_ui->installed->selectedItems();

    for (int i = 0; i < list.size(); i++) {
        list[i]->setIcon(QIcon(":/disabled.png"));
        std::string name = list[i]->text().toStdString();

        addInstruction(
                    new UninstallPackageInstruction(
                        m_client,
                        findInstalledPackage(name),
                        false
                        )
                    );
    }
}

void SoftwareProviderPlugin::updateList()
{
    std::string filter = m_ui->filter_line->text().toStdString();

    m_ui->installed->clear();
    for (unsigned int i = 0; i < m_installed.size(); i++) {
        std::string name, tmp = name = getPackageName(m_installed[i]);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        QListWidgetItem *item = new QListWidgetItem(tmp.c_str());
        if (name.find(filter) != std::string::npos)
            m_ui->installed->addItem(item);
        // TODO
        int pos = findInstruction(IInstruction::SOFTWARE, "", pos);
    }
}

void SoftwareProviderPlugin::updatePackage()
{
    QList<QListWidgetItem*> list = m_ui->installed->selectedItems();

    for (int i = 0; i < list.size(); i++) {
        list[i]->setIcon(QIcon(":/state_changed.png"));
        addInstruction(
                    new UpdatePackageInstruction(
                        m_client,
                        findInstalledPackage(list[i]->text().toStdString())
                        )
                    );
    }
}

void SoftwareProviderPlugin::verifyPackage()
{
    QList<QListWidgetItem*> list = m_ui->installed->selectedItems();

    for (int i = 0; i < list.size(); i++) {
//        list[i]->setIcon(QIcon(":/state_changed.png"));
        std::string name = list[i]->text().toStdString();

        addInstruction(
                    new VerifyPackageInstruction(
                        m_client,
                        findInstalledPackage(name)
                        )
                    );

        m_verify.push_back(name);
        // TODO insert into vector, then check when refreshed
    }
}

Q_EXPORT_PLUGIN2(softwareProvider, SoftwareProviderPlugin)
