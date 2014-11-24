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

#ifndef CODEDIALOG_H
#define CODEDIALOG_H

#include <QDialog>

namespace Ui
{
class CodeDialog;
}

/**
 * @brief The CodeDialog class
 *
 * Show text in dialog.
 */
class CodeDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::CodeDialog *m_ui;

    void initEditor();

private slots:
    void textChanged();

public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit CodeDialog(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~CodeDialog();

    /**
     * @brief Set text of dialog
     * @param text
     * @param move_to_end -- whether to move cursor to end
     */
    void setText(std::string text);
};

#endif // CODEDIALOG_H
