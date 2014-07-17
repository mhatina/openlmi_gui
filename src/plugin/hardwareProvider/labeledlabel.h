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

#ifndef LABELEDLABEL_H
#define LABELEDLABEL_H

#include <QWidget>

namespace Ui {
class LabeledLabel;
}

class LabeledLabel : public QWidget
{
    Q_OBJECT

private:
    Ui::LabeledLabel *m_ui;

public:
    explicit LabeledLabel(std::string label, std::string text);
    ~LabeledLabel();

    int getLabelWidth();
    void setAlignment(int alignment);
    void setVerticalAlignment(Qt::Alignment alignment);
};

#endif // LABELEDLABEL_H
