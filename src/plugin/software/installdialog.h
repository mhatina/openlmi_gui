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

#ifndef INSTALLDIALOG_H
#define INSTALLDIALOG_H

#include "lmiwbem_client.h"

#include <Pegasus/Common/CIMInstance.h>
#include <QDialog>

namespace Ui
{
class InstallDialog;
}

class InstallDialog : public QDialog
{
    Q_OBJECT

private:
    CIMClient *m_client;
    Pegasus::Array<Pegasus::CIMInstance> m_packages;
    Ui::InstallDialog *m_ui;

    Pegasus::CIMInstance findPackage(std::string package_name);


public:
    explicit InstallDialog(CIMClient *client, QWidget *parent = 0);
    ~InstallDialog();

    std::vector<Pegasus::CIMInstance> getPackages();
    void fetchPackages();

private slots:
    void displayData();
    void onSearchButtonClicked();

signals:
    void haveData();
};

#endif // INSTALLDIALOG_H
