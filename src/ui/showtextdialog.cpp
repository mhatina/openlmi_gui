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

#include "config.h"
#include "logger.h"
#include "showtextdialog.h"
#include "ui_showtextdialog.h"

#include <QFile>

ShowTextDialog::ShowTextDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ShowTextDialog)
{
    Logger::getInstance()->debug("ShowTextDialog::ShowTextDialog(QWidget *parent)");
    m_ui->setupUi(this);
}

ShowTextDialog::~ShowTextDialog()
{
    Logger::getInstance()->debug("ShowTextDialog::~ShowTextDialog()");
    delete m_ui;
}

void ShowTextDialog::setText(std::string text, bool move_to_end)
{
    Logger::getInstance()->debug("ShowTextDialog::setText(std::string text, bool move_to_end)");
    m_ui->text_edit->setText(text.c_str());
    if (move_to_end)
        m_ui->text_edit->moveCursor(QTextCursor::End);
}

void ShowTextDialog::setTitle(std::string title)
{
    Logger::getInstance()->debug("ShowTextDialog::setTitle(std::string title)");
    setWindowTitle(title.c_str());
}
