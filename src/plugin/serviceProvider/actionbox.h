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

#ifndef ACTIONBOX_H
#define ACTIONBOX_H

#include "serviceprovider.h"

#include <QWidget>

namespace Ui {
class ActionBox;
}

/**
 * @brief The ActionBox class
 *
 * Combobox widget with possible service actions.
 */
class ActionBox : public QWidget
{
    Q_OBJECT    

public:
    /**
     * @brief Constructor
     * @param serv_name -- name of service
     */
    ActionBox(std::string serv_name);
    /**
     * @brief Destructor
     */
    ~ActionBox();

private:
    std::string m_name;
    Ui::ActionBox *m_ui;

private slots:
    void currentChanged(int i);

signals:
    /**
     * @brief Signal emitted after selecting action
     * @param name -- name of service
     * @param action -- selected action
     */
    void performAction(std::string name, e_action action);
};

#endif // ACTIONBOX_H
