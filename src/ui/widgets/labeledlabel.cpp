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

#include "labeledlabel.h"
#include "ui_labeledlabel.h"

LabeledLabel::LabeledLabel(std::string label, std::string text) :
    QWidget(),
    m_ui(new Ui::LabeledLabel)
{
    m_ui->setupUi(this);
    m_ui->label->setText(label.c_str());
    m_ui->text->setText(text.c_str());
    m_ui->text->setWordWrap(true);
    setObjectName("labeledLabel");
}

LabeledLabel::LabeledLabel(std::string label, QString text) :
    QWidget(),
    m_ui(new Ui::LabeledLabel)
{
    m_ui->setupUi(this);
    m_ui->label->setText(label.c_str());
    m_ui->text->setText(text);
    m_ui->text->setWordWrap(true);
    setObjectName("labeledLabel");
}

LabeledLabel::~LabeledLabel()
{
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
}

int LabeledLabel::getLabelWidth()
{
    return m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
}

void LabeledLabel::setAlignment(int alignment)
{
    int tmp = m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
    if (alignment - tmp > 0) {
        m_ui->label->setMinimumWidth(alignment);
    }
}

void LabeledLabel::setVerticalAlignment(Qt::Alignment alignment)
{
    m_ui->label->setAlignment(alignment);
}
