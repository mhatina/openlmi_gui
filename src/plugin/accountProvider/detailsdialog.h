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

#ifndef DETAILSDIALOG_H
#define DETAILSDIALOG_H

#include "instructions/instruction.h"
#include "labeledlineedit.h"

#include <Pegasus/Common/CIMInstance.h>
#include <QDialog>
#include <QTableWidgetItem>

const char* const key_property[] = {
    "CreationClassName",
    "Name",
    "SystemCreationClassName",
    "SystemName"
};

namespace Ui {
class DetailsDialog;
}

class DetailsDialog : public QDialog
{
    Q_OBJECT

private:    
    bool m_changes_enabled;
    Pegasus::CIMInstance m_instance;
    std::map<std::string, std::string> m_changes;
    Ui::DetailsDialog *m_ui;

    bool isKeyProperty(const char* property);
    std::string insertSpaces(std::string text);

public:
    explicit DetailsDialog(QWidget *parent = 0);
    ~DetailsDialog();

    std::map<std::string, std::string> getChanges();
    void alterProperties(std::vector<IInstruction*> instructions);
    void setInstance(Pegasus::CIMInstance instance);

private slots:
    void itemChanged(LabeledLineEdit *item);
};

#endif // DETAILSDIALOG_H
