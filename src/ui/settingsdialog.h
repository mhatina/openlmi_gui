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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "settings/isettings.h"

#include <map>
#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <vector>

namespace Ui
{
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SettingsDialog *m_ui;
    std::vector<ISettings *> m_settings;

    bool checkState(QObject *box)
    {
        if (qobject_cast<QCheckBox *>(box)) {
            return (((QCheckBox *) box)->checkState() == Qt::Checked);
        }
        return false;
    }

    QString text(QWidget *line)
    {
        if (qobject_cast<QLineEdit *>(line)) {
            return ((QLineEdit *) line)->text();
        }
        return QString();
    }

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void addItem(ISettings *item);
    void deleteItem(ISettings *item);
    ISettings *findItem(std::string title);

    void load();
    void save();

    template<typename T, typename W>
    T value(std::string value_name)
    {
        T ret;

        for (unsigned int i = 0; i < m_settings.size(); i++) {
            W widget = m_settings[i]->findChild<W>(value_name.c_str());
            if (widget == NULL) {
                continue;
            }

            std::string type = widget->metaObject()->className();
            std::stringstream ss;

            if (type == "QCheckBox") {
                ss << checkState(widget);
            } else if (type == "QLineEdit") {
                ss << text(widget).toStdString();
            }
            ss >> ret;
        }

        return ret;
    }


private slots:
    void change();
    void updateList(QString text);
};

#endif // SETTINGSDIALOG_H
