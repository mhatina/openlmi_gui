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

#ifndef SHOWLOGDIALOG_H
#define SHOWLOGDIALOG_H

#include <QDialog>

namespace Ui {
class ShowTextDialog;
}

/**
 * @brief The ShowTextDialog class
 *
 * Show text in dialog.
 */
class ShowTextDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit ShowTextDialog(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~ShowTextDialog();

    /**
     * @brief Set text of dialog
     * @param text
     * @param move_to_end -- whether to move cursor to end
     */
    void setText(std::string text, bool move_to_end = true);
    /**
     * @brief Set title of dialog
     * @param title
     */
    void setTitle(std::string title);

private:
    Ui::ShowTextDialog *m_ui;
};

#endif // SHOWLOGDIALOG_H
