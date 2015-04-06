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

#ifndef LABELEDLINEEDIT_H
#define LABELEDLINEEDIT_H

#include <string>
#include <QLineEdit>
#include <QWidget>

namespace Ui
{
class LabeledLineEdit;
}

class LabeledLineEdit : public QWidget
{
    Q_OBJECT

private:
    std::string m_text;
    Ui::LabeledLineEdit *m_ui;

public:
    explicit LabeledLineEdit(std::string objectName, std::string label,
                             std::string text, bool key = false);
    ~LabeledLineEdit();

    int getLabelWidth();
    std::string getLabelText();
    std::string getObjectName();
    std::string getText();
    void setAlignment(int alignment);
    void setLabelText(std::string text);
    void setReadOnly(bool state);
    void setText(std::string text, bool emit_changes = true);

public slots:
    void itemChanged();
    void textChanged();

signals:
    void itemChanged(LabeledLineEdit *item);
};

#endif // LABELEDLINEEDIT_H
