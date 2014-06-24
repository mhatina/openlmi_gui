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

#ifndef NEWUSERINSTRUCTION_H
#define NEWUSERINSTRUCTION_H

#include "instructions/accountinstruction.h"
#include "dialogs/newuserdialog.h"

/**
 * @brief The NewUserInstruction class
 *
 * Create new user.
 */
class NewUserInstruction : public AccountInstruction
{
private:
    NewUserDialog *m_dialog;

public:
    /**
     * @brief Constructor
     * @param client -- see CIMClient
     * @param dialog -- see NewUserDialog
     */
    NewUserInstruction(CIMClient *client, NewUserDialog *dialog);
    IInstruction::Subject getSubject();
    std::string toString();
    void run();
};

#endif // NEWUSERINSTRUCTION_H
