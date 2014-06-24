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

#ifndef MASTERPASSWDDIALOG_H
#define MASTERPASSWDDIALOG_H

#include <QDialog>

namespace Ui {
class MasterPasswdDialog;
}

/**
 * @brief The MasterPasswdDialog class
 *
 * Not used yet.
 */
class MasterPasswdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MasterPasswdDialog(QWidget *parent = 0);
    ~MasterPasswdDialog();

private:
    Ui::MasterPasswdDialog *ui;
};

#endif // MASTERPASSWDDIALOG_H
