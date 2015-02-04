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

#ifndef PROVIDERWIDGET_H
#define PROVIDERWIDGET_H

#include "lmi_string.h"
#include "logger.h"

#include <QComboBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>

namespace Ui
{
class ProviderWidget;
}

/**
 * @brief The ProviderWidget class
 */
class ProviderWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::ProviderWidget *m_ui;

public:
    /**
     * @brief Constructor
     * @param parent -- parent of widget
     */
    explicit ProviderWidget(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~ProviderWidget();
    /**
     * @brief Getter
     * @return Tab widget (provider display its data here)
     */
    QTabWidget *getTabWidget();
    void setTitle(String title);
};

#endif // PROVIDERWIDGET_H
