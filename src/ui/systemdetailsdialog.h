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

#include <QDialog>

#define DEFAULT_TEXT "N/A"

namespace Ui {
class SystemDetailsDialog;
}

class SystemDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemDetailsDialog(QWidget *parent = 0);
    ~SystemDetailsDialog();

    void setDomain(std::string domain);
    void setIpv4(std::string ipv4);
    void setIpv6(std::string ipv6);
    void setMac(std::string mac);
    void reset();

protected:
    void leaveEvent(QEvent *event);

private:
    Ui::SystemDetailsDialog *m_ui;
};

#endif // SYSTEMDETAILSDIALOG_H
