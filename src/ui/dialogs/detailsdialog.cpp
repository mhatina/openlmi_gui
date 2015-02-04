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

#include "cimvalue.h"
#include "detailsdialog.h"
#include "logger.h"
#include "widgets/labeledlineedit.h"
#include "ui_detailsdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

bool DetailsDialog::isKeyProperty(const char *property)
{
    Logger::getInstance()->debug("DetailsDialog::isKeyProperty(const char *property)");
    int cnt = sizeof(key_property) / sizeof(key_property[0]);
    for (int i = 0; i < cnt; i++) {
        if (strcmp(key_property[i], property) == 0) {
            return true;
        }
    }

    return false;
}

String DetailsDialog::insertSpaces(String text)
{
    Logger::getInstance()->debug("DetailsDialog::insertSpaces(String text)");
    for (unsigned int i = 1; i < text.length(); i++) {
        if (isupper(text[i]) && islower(text[i - 1])) {
            text.insert(i, " ");
            i++;
        }
    }

    return text;
}

DetailsDialog::DetailsDialog(String title, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DetailsDialog)
{
    Logger::getInstance()->debug("DetailsDialog::DetailsDialog(String title, QWidget *parent)");
    m_ui->setupUi(this);
    setWindowTitle(title);
    m_ui->key_label->setStyleSheet("QLabel { background-color : pink;}");
    m_ui->changed_label->setStyleSheet("QLabel { background-color : yellow;}");
    setWindowFlags(Qt::Popup);
}

DetailsDialog::~DetailsDialog()
{
    Logger::getInstance()->debug("DetailsDialog::~DetailsDialog()");
    delete m_ui;
}

std::map<String, String> DetailsDialog::getChanges()
{
    Logger::getInstance()->debug("DetailsDialog::getChanges()");
    return m_changes;
}

void DetailsDialog::alterProperties(std::map<String, String>
                                    instructions)
{
    Logger::getInstance()->debug("DetailsDialog::alterProperties(std::map<String, String> instructions)");
    std::map<String, String>::iterator it;
    for (it = instructions.begin(); it != instructions.end(); it++) {
        LabeledLineEdit *line;
        if ((line = findChild<LabeledLineEdit *>(it->first)) != NULL) {
            line->textChanged();
            line->setText(it->second);
        }
    }
}

void DetailsDialog::hideCancelButton()
{
    Logger::getInstance()->debug("DetailsDialog::hideCancelButton()");
    m_ui->button_box->button(QDialogButtonBox::Cancel)->hide();
}

void DetailsDialog::setValues(Pegasus::CIMInstance instance, bool disableAll)
{
    Logger::getInstance()->debug("DetailsDialog::setValues(Pegasus::CIMInstance instance, bool disableAll)");
    std::map<String, String> values;
    int cnt = instance.getPropertyCount();
    for (int i = 0; i < cnt; i++) {
        String object_name = CIMValue::to_string(instance.getProperty(i).getName().getString());
        String str_value = CIMValue::get_property_value(instance, object_name);
        values[object_name] = str_value;
    }

    setValues(values, disableAll);
}

void DetailsDialog::setValues(std::map<String, String> values,
                              bool disableAll)
{
    Logger::getInstance()->debug("DetailsDialog::setValues(std::map<String, String> values, bool disableAll)");
    m_changes_enabled = false;
    std::map<String, String>::iterator it;

    std::vector<LabeledLineEdit *> lines;
    int max_width = 0;
    for (it = values.begin(); it != values.end(); it++) {
        String object_name = it->first;
        String str_name = insertSpaces(object_name);
        String str_value = it->second;
        QWidget *widget_area = findChild<QWidget *>("widget_area");
        QLayout *layout = widget_area->layout();

        bool key = isKeyProperty(object_name);
        LabeledLineEdit *widget = new LabeledLineEdit(object_name, str_name, str_value,
                key);
        widget->setReadOnly(disableAll | key);
        if (max_width < widget->getLabelWidth()) {
            max_width = widget->getLabelWidth();
        }
        connect(
            widget,
            SIGNAL(itemChanged(LabeledLineEdit *)),
            this,
            SLOT(itemChanged(LabeledLineEdit *)));
        layout->addWidget(widget);
        lines.push_back(widget);
    }
    int cnt = lines.size();
    for (int i = 0; i < cnt; i++) {
        lines[i]->setAlignment(max_width);
    }

    m_changes_enabled = true;
}


void DetailsDialog::itemChanged(LabeledLineEdit *item)
{
    Logger::getInstance()->debug("DetailsDialog::itemChanged(LabeledLineEdit *item)");
    if (!m_changes_enabled) {
        return;
    }

    String property_name = item->getObjectName();
    m_changes[property_name] = item->getText();
}
