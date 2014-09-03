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

#include "installdialog.h"
#include "lmiwbem_value.h"
#include "logger.h"
#include "ui_installdialog.h"

#include <boost/thread.hpp>

Pegasus::CIMInstance InstallDialog::findPackage(std::string package_name)
{
    Pegasus::CIMInstance instance;
    for (unsigned int i = 0; i < m_packages.size(); i++) {
        Pegasus::Uint32 prop_ind = m_packages[i].findProperty("InstalledSoftware");
        std::string prop = (std::string) m_packages[i].getProperty(prop_ind).getValue().toString().getCString();
        if (prop.find(package_name) != std::string::npos) {
            instance = m_packages[i];
            break;
        }
    }

    return instance;
}

InstallDialog::InstallDialog(CIMClient *client, QWidget *parent) :
    QDialog(parent),
    m_client(client),
    m_ui(new Ui::InstallDialog)
{
    m_ui->setupUi(this);
    m_ui->progress_bar->hide();
    m_ui->progress_bar->setMaximum(0);

    connect(
        m_ui->search_button,
        SIGNAL(clicked()),
        this,
        SLOT(onSearchButtonClicked()));
    connect(
        this,
        SIGNAL(haveData()),
        this,
        SLOT(displayData()));
}

InstallDialog::~InstallDialog()
{
    delete m_ui;
}

std::vector<Pegasus::CIMInstance> InstallDialog::getPackages()
{
    QList<QListWidgetItem*> list = m_ui->list_widget->selectedItems();
    std::vector<Pegasus::CIMInstance> vector;
    for (int i = 0; i < list.size(); i++) {
        vector.push_back(findPackage(list[i]->text().toStdString()));
    }

    return vector;
}

void InstallDialog::onSearchButtonClicked()
{
    Logger::getInstance()->error("Not yet implemented");
    return;

    m_ui->progress_bar->show();
    boost::thread(boost::bind(&InstallDialog::fetchPackages, this));
}

void InstallDialog::fetchPackages()
{
    try {
        m_packages = m_client->enumerateInstances(
                        Pegasus::CIMNamespaceName("root/cimv2"),
                        Pegasus::CIMName("LMI_SoftwareIdentity"),
                        true,       // deep inheritance
                        false,      // local only
                        false,      // include qualifiers
                        false       // include class origin
                        );
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(std::string(ex.getMessage().getCString()));
    }

    emit haveData();
}

void InstallDialog::displayData()
{
    int cnt = m_packages.size();
    for (int i = 0; i < cnt; i++) {
        std::string name = CIMValue::get_property_value(m_packages[i], "ElementName");
        if (name.find(m_ui->search_line->text().toStdString()) != std::string::npos)
            m_ui->list_widget->addItem(new QListWidgetItem(name.c_str()));
    }

    m_ui->progress_bar->hide();

}
