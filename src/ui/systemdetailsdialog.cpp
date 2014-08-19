/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "systemdetailsdialog.h"
#include "ui_systemdetailsdialog.h"

SystemDetailsDialog::SystemDetailsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SystemDetailsDialog)
{
    m_ui->setupUi(this);
}

SystemDetailsDialog::~SystemDetailsDialog()
{
    delete m_ui;
}

void SystemDetailsDialog::setDomain(std::string domain)
{
    m_ui->domain->setText(domain.c_str());
}

void SystemDetailsDialog::setIpv4(std::string ipv4)
{
    m_ui->ipv4->setText(ipv4.c_str());
}

void SystemDetailsDialog::setIpv6(std::string ipv6)
{
    m_ui->ipv6->setText(ipv6.c_str());
}

void SystemDetailsDialog::setMac(std::string mac)
{
    m_ui->mac->setText(mac.c_str());
}

void SystemDetailsDialog::reset()
{
    m_ui->domain->setText(DEFAULT_TEXT);
    m_ui->ipv4->setText(DEFAULT_TEXT);
    m_ui->ipv6->setText(DEFAULT_TEXT);
    m_ui->mac->setText(DEFAULT_TEXT);
}

void SystemDetailsDialog::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    hide();
}
