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

#include "providerwidget.h"
#include "ui_providerwidget.h"

#include <QList>
#include <QTreeWidgetItem>

ProviderWidget::ProviderWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ProviderWidget)
{
    Logger::getInstance()->debug("ProviderWidget::ProviderWidget(QWidget *parent)");
    m_ui->setupUi(this);
}

ProviderWidget::~ProviderWidget()
{
    Logger::getInstance()->debug("ProviderWidget::~ProviderWidget()");
    delete m_ui;
}

QTabWidget *ProviderWidget::getTabWidget()
{
    Logger::getInstance()->debug("ProviderWidget::getTabWidget()");
    return m_ui->tab_widget;
}

void ProviderWidget::setTitle(String title)
{
    m_ui->providers->setTitle(title);
}
