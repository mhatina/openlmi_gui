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

#ifndef MEMBERBOX_H
#define MEMBERBOX_H

#include <QWidget>

namespace Ui
{
class MemberBox;
}

/**
 * @brief The MemberBox class
 *
 * ComboBox with members of group
 */
class MemberBox : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param owner -- group name
     */
    explicit MemberBox(std::string owner);
    /**
     * @brief Destructor
     */
    ~MemberBox();

    /**
     * @brief Find item with text
     * @param text
     * @return index of item
     */
    int findItem(std::string text);
    /**
     * @brief Count of members in combobox
     * @return item count
     */
    int itemCount();
    /**
     * @brief Getter
     * @param i -- index
     * @return item with index @ref i
     */
    std::string getItem(int i);
    /**
     * @brief Add item with text
     * @param text
     */
    void addItem(std::string text);
    /**
     * @brief Delete item with text
     * @param text
     */
    void deleteItem(std::string text);

private:
    Ui::MemberBox *m_ui;
    std::string m_owner;

private slots:
    /**
     * @brief Add user
     *
     * Display @ref GroupMemberBox
     */
    void addUser();
    /**
     * @brief Remove user
     *
     * Display @ref GroupMemberBox
     */
    void removeUser();

signals:
    /**
     * @brief Emitted when on '+' button clicked
     * @param group -- name of group
     */
    void add(std::string group);
    /**
     * @brief Emitted when on '-' button clicked
     * @param group -- name of group
     */
    void remove(std::string group);
};

#endif // MEMBERBOX_H
