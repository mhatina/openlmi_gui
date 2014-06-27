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

#include "labeledlineedit.h"
#include "ui_labeledlineedit.h"

LabeledLineEdit::LabeledLineEdit(std::string objectName, std::string label, std::string text, bool key) :
    QWidget(0),
    m_text(text),
    m_ui(new Ui::LabeledLineEdit)
{
    m_ui->setupUi(this);
    setLabelText(label);
    setText(text);    
    if (key) {
        m_ui->lineEdit->setReadOnly(true);
        m_ui->lineEdit->setStyleSheet("QLineEdit{background: pink;}");
    }


    setObjectName(objectName.c_str());

    connect(
        m_ui->lineEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(textChanged()));
    connect(
        m_ui->lineEdit,
        SIGNAL(editingFinished()),
        this,
        SLOT(itemChanged()));    
}

LabeledLineEdit::~LabeledLineEdit()
{
    delete m_ui;
}

int LabeledLineEdit::getLabelWidth()
{
    return m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
}

std::string LabeledLineEdit::getLabelText()
{
    return m_ui->label->text().toStdString();
}

std::string LabeledLineEdit::getObjectName()
{
    return objectName().toStdString();
}

std::string LabeledLineEdit::getText()
{
    return m_ui->lineEdit->text().toStdString();
}

void LabeledLineEdit::setAlignment(int alignment)
{
    int tmp = m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
    if (alignment - tmp > 0)
        m_ui->label->setMinimumWidth(alignment);
}

void LabeledLineEdit::setLabelText(std::string text)
{
    m_ui->label->setText(text.c_str());
}

void LabeledLineEdit::setText(std::string text)
{
    m_text = text;
    m_ui->lineEdit->setText(text.c_str());
}

void LabeledLineEdit::itemChanged()
{
    if (m_ui->lineEdit->text().toStdString() != m_text)
        emit itemChanged(this);
}

void LabeledLineEdit::textChanged()
{
    m_ui->lineEdit->setStyleSheet("QLineEdit{background: yellow;}");
}
