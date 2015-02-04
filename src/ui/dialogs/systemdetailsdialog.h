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

#ifndef SYSTEMDETAILSDIALOG_H
#define SYSTEMDETAILSDIALOG_H

#include "lmi_string.h"

#include <QDialog>

#define DEFAULT_TEXT "N/A"

namespace Ui
{
class SystemDetailsDialog;
}

class SystemDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemDetailsDialog(QWidget *parent = 0);
    ~SystemDetailsDialog();

    void setDomain(String domain);
    void setIpv4(String ipv4);
    void setIpv6(String ipv6);
    void setMac(String mac);
    void reset();

protected:
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::SystemDetailsDialog *m_ui;
};

#endif // SYSTEMDETAILSDIALOG_H
