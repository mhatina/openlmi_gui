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
#include "installdialog.h"
#include "instructions/disablerepoinstruction.h"
#include "instructions/enablerepoinstruction.h"
#include "instructions/installpackageinstruction.h"
#include "instructions/uninstallpackageinstruction.h"
#include "instructions/updatepackageinstruction.h"
#include "instructions/verifypackageinstruction.h"
#include "lmiwbem_value.h"
#include "software.h"
#include "ui_software.h"

#include <algorithm>
#include <boost/thread.hpp>
#include <QMenu>
#include <sstream>

Pegasus::CIMInstance SoftwarePlugin::findInstalledPackage(
    std::string package_name)
{
    Pegasus::CIMInstance instance;
    for (unsigned int i = 0; i < m_installed.size(); i++) {
        Pegasus::Uint32 prop_ind = m_installed[i].findProperty("InstalledSoftware");
        std::string prop = (std::string) m_installed[i].getProperty(
                               prop_ind).getValue().toString().getCString();
        if (prop.find(package_name) != std::string::npos) {
            instance = m_installed[i];
            break;
        }
    }

    return instance;
}

Pegasus::CIMInstance SoftwarePlugin::findRepo(std::string repo_name)
{
    Pegasus::CIMInstance instance;
    for (unsigned int i = 0; i < m_repos.size(); i++) {
        Pegasus::Uint32 prop_ind = m_repos[i].findProperty("Caption");
        std::string prop = (std::string) m_repos[i].getProperty(
                               prop_ind).getValue().toString().getCString();
        if (prop.find(repo_name) != std::string::npos) {
            instance = m_repos[i];
            break;
        }
    }

    return instance;
}

std::string SoftwarePlugin::getPackageName(Pegasus::CIMInstance package)
{
    Pegasus::Uint32 prop_ind = package.findProperty("InstalledSoftware");
    Pegasus::CIMProperty property = package.getProperty(prop_ind);

    std::string name;
    if (property.getValue().isNull()) {
        return "";
    }

    name = property.getValue().toString().getCString();

    int ch = name.rfind(":", name.rfind(":") - 1);
    name = name.substr(ch + 1, name.length() - ch - 2);
    return name;
}

void SoftwarePlugin::fetchPackageInfo(Pegasus::CIMInstance instance)
{
    Pegasus::CIMInstance package;

    try {
        Pegasus::Uint32 prop_ind = instance.findProperty("InstalledSoftware");
        Pegasus::CIMValue value = instance.getProperty(prop_ind).getValue();

        Pegasus::CIMObjectPath path;
        if (!value.isNull()) {
            value.get(path);
        }

        package = m_client->getInstance(
                      Pegasus::CIMNamespaceName("root/cimv2"),
                      path
                  );
    } catch (Pegasus::Exception &ex) {
        emit refreshProgress(Engine::ERROR, this);
        return;
    }

    emit havePackageDetails(package);
}

void SoftwarePlugin::initContextMenu()
{
    Logger::getInstance()->debug("SoftwarePlugin::initContextMenu()");
    QAction *action;

    m_context_menu = new QMenu(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    action = m_context_menu->addAction("Show/hide buttons");
    action->setObjectName("show_hide_buttons_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showButtons()));

    m_package_context_menu = new QMenu(m_ui->installed);
    m_ui->installed->setContextMenuPolicy(Qt::CustomContextMenu);

    action = m_package_context_menu->addAction("Verify");
    action->setObjectName("verify_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(verifyPackage()));

    action = m_package_context_menu->addAction("Install");
    action->setObjectName("install_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(installPackage()));

    action = m_package_context_menu->addAction("Uninstall");
    action->setObjectName("uninstall_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(uninstallPackage()));

    action = m_package_context_menu->addAction("Update");
    action->setObjectName("update_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(updatePackage()));

    m_package_context_menu->addSeparator();

    action = m_package_context_menu->addAction("Show details");
    action->setObjectName("show_details_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(getPackageDetail()));

    m_package_context_menu->addSeparator();

    action = m_package_context_menu->addAction("Show/hide buttons");
    action->setObjectName("show_hide_buttons_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showButtons()));

    m_repo_context_menu = new QMenu(m_ui->repos);
    m_ui->repos->setContextMenuPolicy(Qt::CustomContextMenu);

    action = m_repo_context_menu->addAction("Enable repo");
    action->setObjectName("enable_repo_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(enableRepo()));

    action = m_repo_context_menu->addAction("Disable repo");
    action->setObjectName("disable_repo_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(disableRepo()));

    m_repo_context_menu->addSeparator();

    action = m_repo_context_menu->addAction("Show details");
    action->setObjectName("show_details_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showRepoDetail()));

    m_repo_context_menu->addSeparator();

    action = m_repo_context_menu->addAction("Show/hide buttons");
    action->setObjectName("show_hide_buttons_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(showButtons()));
}

SoftwarePlugin::SoftwarePlugin() :
    IPlugin(),
    m_changes_enabled(false),
    m_ui(new Ui::SoftwarePlugin)
{
    m_ui->setupUi(this);
    m_ui->filter_box->hide();
    setPluginEnabled(false);

    initContextMenu();
    m_ui->enable_repo_button->setVisible(false);
    m_ui->disable_repo_button->setVisible(false);
    m_ui->install_button->setVisible(false);
    m_ui->verify_package_button->setVisible(false);
    m_ui->uninstall_button->setVisible(false);
    m_ui->update_button->setVisible(false);

    connect(
        m_ui->installed,
        SIGNAL(itemDoubleClicked(QListWidgetItem *)),
        this,
        SLOT(getPackageDetail(QListWidgetItem *)));
    connect(m_ui->repos,
            SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this,
            SLOT(showRepoDetail(QListWidgetItem *)));
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
    connect(
        this,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showContextMenu(QPoint)));
    connect(
        m_ui->installed,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showPackageContextMenu(QPoint)));
    connect(
        m_ui->repos,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showRepoContextMenu(QPoint)));
}

SoftwarePlugin::~SoftwarePlugin()
{
    delete m_ui;
}

std::string SoftwarePlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

std::string SoftwarePlugin::getLabel()
{
    return "Software";
}

std::string SoftwarePlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << getLabel() << ": " << m_installed.size() << " installed package(s), "
       << m_repos.size() << " repository(ies) shown";
    return ss.str();
}

void SoftwarePlugin::getData(std::vector<void *> *data)
{
    m_still_refreshing = true;

    std::string filter = m_ui->filter_line->text().toStdString();

    try {
        Pegasus::Array<Pegasus::CIMInstance> installed;
        Pegasus::Array<Pegasus::CIMInstance> repos;
        Pegasus::Array<Pegasus::CIMInstance> verify;

        // repos
        repos = enumerateInstances(
                    Pegasus::CIMNamespaceName("root/cimv2"),
                    Pegasus::CIMName("LMI_SoftwareIdentityResource"),
                    true,       // deep inheritance
                    false,      // local only
                    false,      // include qualifiers
                    false       // include class origin
                );

        std::vector<void *> *tmp = new std::vector<void *>();
        for (unsigned int i = 0; i < repos.size(); i++) {
            tmp->push_back(new Pegasus::CIMInstance(repos[i]));
        }
        emit doneFetchingData(tmp);

        // installed packages
        installed = enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("LMI_InstalledSoftwareIdentity"),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                    );

        tmp = new std::vector<void *>();
        for (unsigned int i = 0; i < installed.size(); i++) {
            Pegasus::Uint32 prop_ind = installed[i].findProperty("InstalledSoftware");
            Pegasus::CIMProperty property = installed[i].getProperty(prop_ind);

            std::string name;
            if (!property.getValue().isNull()) {
                name = CIMValue::to_std_string(property.getValue());
            }

            if (filter.empty()) {
                tmp->push_back(new Pegasus::CIMInstance(installed[i]));
            } else if (name.find(filter) != std::string::npos) {
                tmp->push_back(new Pegasus::CIMInstance(installed[i]));
            }
        }
        emit doneFetchingData(tmp);

        // verification job
        if (!m_verify.empty()) {
            verify = enumerateInstances(
                         Pegasus::CIMNamespaceName("root/cimv2"),
                         Pegasus::CIMName("LMI_SoftwareVerificationJob"),
                         true,       // deep inheritance
                         false,      // local only
                         false,      // include qualifiers
                         false       // include class origin
                     );

            for (unsigned int i = 0; i < verify.size(); i++) {
                data->push_back(new Pegasus::CIMInstance(verify[i]));
            }
        }
    } catch (Pegasus::Exception &ex) {
        m_still_refreshing = false;
        emit doneFetchingData(NULL, CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    m_still_refreshing = false;
    emit doneFetchingData(data);
}

void SoftwarePlugin::clear()
{
    m_changes_enabled = false;

    m_ui->repos->clear();
    m_ui->installed->clear();

    m_changes_enabled = true;

}

void SoftwarePlugin::fillTab(std::vector<void *> *data)
{
    m_changes_enabled = false;

    try {
        for (unsigned int i = 0; i < data->size(); i++) {
            Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[i]);

            Pegasus::Uint32 prop_ind = instance->findProperty("InstalledSoftware");
            if (prop_ind != Pegasus::PEG_NOT_FOUND &&
                instance->getProperty(prop_ind).getValue().toString().find("LMI:LMI_SoftwareIdentity") != Pegasus::PEG_NOT_FOUND) {

                Pegasus::CIMProperty property = instance->getProperty(prop_ind);
                std::string name = (std::string) property.getValue().toString().getCString();

                int ch = name.rfind(":", name.rfind(":") - 1);
                name = name.substr(ch + 1, name.length() - ch - 2);

                m_ui->installed->addItem(new QListWidgetItem(name.c_str()));

                m_installed.push_back(*instance);
            } else if (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_SoftwareIdentityResource") {
                QListWidgetItem *item = new QListWidgetItem(
                    CIMValue::get_property_value(*instance, "Caption").c_str());
                item->setIcon(QIcon(CIMValue::get_property_value(*instance,
                                    "EnabledState") == "2" ? ":/enabled.png" : ":/disabled.png"));
                m_ui->repos->addItem(item);

                m_repos.push_back(*instance);
            } else if (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_SoftwareVerificationJob") {
                // TODO
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->critical(CIMValue::to_std_string(ex.getMessage()));
        return;
    }

    for (unsigned int i = 0; i < data->size(); i++) {
        delete ((Pegasus::CIMInstance *) (*data)[i]);
    }

    disableRepoButtons();
    disablePackageButtons();

    m_changes_enabled = true;
}

void SoftwarePlugin::disablePackageButtons()
{
    bool empty = m_ui->installed->selectedItems().empty();

    m_ui->verify_package_button->setEnabled(!empty);
    m_ui->uninstall_button->setEnabled(!empty);
    m_ui->update_button->setEnabled(!empty);
}

void SoftwarePlugin::disableRepo()
{
    if (m_ui->repos->selectedItems().empty()) {
        return;
    }

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

void SoftwarePlugin::disableRepoButtons()
{
    bool empty = m_ui->repos->selectedItems().empty();

    m_ui->enable_repo_button->setEnabled(!empty);
    m_ui->disable_repo_button->setEnabled(!empty);
}

void SoftwarePlugin::enableRepo()
{
    if (m_ui->repos->selectedItems().empty()) {
        return;
    }

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

void SoftwarePlugin::getPackageDetail()
{
    QList<QListWidgetItem *> list = m_ui->installed->selectedItems();
    if (list.empty()) {
        return;
    }

    getPackageDetail(list[0]);
}

void SoftwarePlugin::getPackageDetail(QListWidgetItem *item)
{
    if (item != NULL) {
        std::string id = item->text().toStdString();
        emit refreshProgress(Engine::NOT_REFRESHED, this, "Downloading package data: " + id);

        unsigned int cnt = m_installed.size();
        for (unsigned int i = 0; i < cnt; i++) {
            if (getPackageName(m_installed[i]) != id) {
                continue;
            }

            boost::thread(boost::bind(&SoftwarePlugin::fetchPackageInfo, this,
                                      m_installed[i]));
            break;
        }
    }
}

void SoftwarePlugin::showButtons()
{
    bool visible = m_ui->enable_repo_button->isVisible();
    m_ui->enable_repo_button->setVisible(!visible);
    m_ui->disable_repo_button->setVisible(!visible);
    m_ui->install_button->setVisible(!visible);
    m_ui->verify_package_button->setVisible(!visible);
    m_ui->uninstall_button->setVisible(!visible);
    m_ui->update_button->setVisible(!visible);
}

void SoftwarePlugin::showContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("ServicePlugin::showContextMenu(QPoint pos)");
    QPoint globalPos = mapToGlobal(pos);
    m_context_menu->popup(globalPos);
}

void SoftwarePlugin::showPackageContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("ServicePlugin::showPackageContextMenu(QPoint pos)");
    QPoint globalPos = m_ui->installed->mapToGlobal(pos);
    m_package_context_menu->popup(globalPos);
}

void SoftwarePlugin::showPackageDetail(Pegasus::CIMInstance item)
{
    std::string name = CIMValue::get_property_value(item, "ElementName");
    emit refreshProgress(Engine::REFRESHED, this, "Downloading package data: " + name);

    DetailsDialog dialog("Package details", this);
    dialog.hideCancelButton();
    dialog.setValues(item, true);
    dialog.exec();
}

void SoftwarePlugin::installPackage()
{
    InstallDialog dialog(m_client, this);
    if (dialog.exec()) {
        std::vector<Pegasus::CIMInstance> packages = dialog.getPackages();
        for (unsigned int i = 0; i < packages.size(); i++) {
            addInstruction(
                new InstallPackageInstruction(
                    m_client,
                    packages[i],
                    false));
            m_ui->installed->addItem(new QListWidgetItem(
                                         QIcon(":/enabled.png"),
                                         CIMValue::get_property_value(packages[i], "ElementName").c_str())
                                    );
        }
    }
}

void SoftwarePlugin::showRepoContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("ServicePlugin::showRepoContextMenu(QPoint pos)");
    QPoint globalPos = m_ui->repos->mapToGlobal(pos);
    m_repo_context_menu->popup(globalPos);
}

void SoftwarePlugin::showRepoDetail()
{
    QList<QListWidgetItem *> list = m_ui->repos->selectedItems();
    if (list.empty()) {
        return;
    }

    showRepoDetail(list[0]);
}

void SoftwarePlugin::showRepoDetail(QListWidgetItem *item)
{
    Pegasus::CIMInstance repo;
    for (unsigned int i = 0; i < m_repos.size(); i++) {
        if (item->text().toStdString() == CIMValue::get_property_value(m_repos[i],
                "Caption")) {
            repo = m_repos[i];
            break;
        }
    }

    DetailsDialog dialog("Repo details", this);
    dialog.hideCancelButton();
    dialog.setValues(repo, true);
    dialog.exec();
}

void SoftwarePlugin::uninstallPackage()
{
    QList<QListWidgetItem *> list = m_ui->installed->selectedItems();

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

void SoftwarePlugin::updateList()
{
    std::string filter = m_ui->filter_line->text().toStdString();

    m_ui->installed->clear();
    for (unsigned int i = 0; i < m_installed.size(); i++) {
        std::string name, tmp = name = getPackageName(m_installed[i]);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        QListWidgetItem *item = new QListWidgetItem(tmp.c_str());
        if (name.find(filter) != std::string::npos) {
            m_ui->installed->addItem(item);
        }
    }

    unsigned int pos = 0;
    while (pos < m_instructions.size()) {
        pos = findInstruction(IInstruction::SOFTWARE, "", pos);

        QList<QListWidgetItem *> list =
            m_ui->installed->findItems(
                ((SoftwareInstruction *) m_instructions[pos])->getName().c_str(),
                Qt::MatchExactly);
        if (list.empty()) {
            pos++;
            continue;
        }

        if (m_instructions[pos]->getInstructionName() == "install_package") {
            list[0]->setIcon(QIcon(":/enabled.png"));
        } else if (m_instructions[pos]->getInstructionName() == "uninstall_package") {
            list[0]->setIcon(QIcon(":/disabled.png"));
        } else if (m_instructions[pos]->getInstructionName() == "update_package") {
            list[0]->setIcon(QIcon(":/state_changed.png"));
        }
        pos++;
    }
}

void SoftwarePlugin::updatePackage()
{
    QList<QListWidgetItem *> list = m_ui->installed->selectedItems();

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

void SoftwarePlugin::verifyPackage()
{
    QList<QListWidgetItem *> list = m_ui->installed->selectedItems();

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

Q_EXPORT_PLUGIN2(software, SoftwarePlugin)
