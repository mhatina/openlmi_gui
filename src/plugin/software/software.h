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

#ifndef SOFTWARE_H
#define SOFTWARE_H

#include "lmi_string.h"
#include "plugin.h"

#include <QListWidgetItem>
#include <QtPlugin>

namespace Ui
{
class SoftwarePlugin;
}

class SoftwarePlugin : public Engine::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(Engine::IPlugin)

private:
    bool m_changes_enabled;
    QMenu *m_context_menu;
    QMenu *m_package_context_menu;
    QMenu *m_repo_context_menu;
    std::vector<Pegasus::CIMInstance> m_installed;
    std::vector<Pegasus::CIMInstance> m_repos;
    std::vector<String> m_verify;
    Ui::SoftwarePlugin *m_ui;

    Pegasus::CIMInstance findInstalledPackage(String package_name);
    Pegasus::CIMInstance findRepo(String repo_name);
    String getPackageName(Pegasus::CIMInstance package);
    void fetchPackageInfo(Pegasus::CIMInstance instance);
    void initContextMenu();

public:
    explicit SoftwarePlugin();
    ~SoftwarePlugin();
    virtual String getInstructionText();
    virtual String getLabel();
    virtual String getRefreshInfo();
    virtual void clear();
    virtual void fillTab(std::vector<void *> *data);
    virtual void getData(std::vector<void *> *data);

private slots:
    void disablePackageButtons();
    void disableRepo();
    void disableRepoButtons();
    void enableRepo();
    void getPackageDetail();
    void getPackageDetail(QListWidgetItem *item);
    void installPackage();
    void showButtons();
    void showContextMenu(QPoint pos);
    void showPackageContextMenu(QPoint pos);
    void showPackageDetail(Pegasus::CIMInstance item);
    void showRepoContextMenu(QPoint pos);
    void showRepoDetail();
    void showRepoDetail(QListWidgetItem *item);
    void uninstallPackage();
    void updateList();
    void updatePackage();
    void verifyPackage();

signals:
    void havePackageDetails(Pegasus::CIMInstance item);
};

#endif // SOFTWARE_H
