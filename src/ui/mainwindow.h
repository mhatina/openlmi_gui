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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "showtextdialog.h"
#include "widgets/pctreewidget.h"
#include "widgets/providerwidget.h"

#include <QMainWindow>

#define WINDOW_TITLE "LMI Command Center"
#define PROVIDER_BOX_TITLE "System: "

/**
 * @brief Struct representation of push button in toolbar
 */
typedef struct {
    const char *object_name;
    const char *icon_path;
    const char *tooltip;
    const char *shortcut;
    bool disabled;
    bool checkable;
} button;

const button buttons[] = {
    {"add_button", ":/add.png", "Add", "", false, false},
    {"remove_button", ":/remove.png", "Remove", "", false, false},
    {"discover_button", ":/discover.png", "Discover", "", false, false},
    {"delete_passwd_button", ":/change-password.png", "Delete \npassword", "", true, false},
    {"", "", "", "", true, false},
    {"refresh_button", ":/refresh.png", "Refresh", "Ctrl+R", true, false},
    {"stop_refresh_button", ":/stop-refresh.png", "Stop refresh", "Ctrl+S", true, false},
    {"cancel_button", ":/cancel.png", "Cancel", "", true, false},
    {"save_button", ":/save.png", "Save", "", true, false},
    {"save_as_button", ":/save-as.png", "Save as...", "", true, false},
    {"apply_button", ":/apply.png", "Apply", "", true, false},
    {"", "", "", "", true, false},
    {"", "", "", "", true, false},
    {"show_code_button", ":/show-code.png", "Show code", "", false, false},
    {"", "", "", "", true, false},
    {"filter_button", ":/filter.png", "Filter", "Ctrl+F", false, true}
};

namespace Ui
{
class MainWindow;
}

/**
 * @brief The MainWindow class
 *
 * Represent main window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QMutex *m_mutex;
    QToolBar *m_toolbar;
    ShowTextDialog m_log_dialog;
    Ui::MainWindow *m_ui;

public:
    /**
     * @brief Constructor
     * @param parent -- parent of dialog
     */
    explicit MainWindow(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~MainWindow();

    /**
     * @brief Getter
     * @return PCTreeWidget
     */
    PCTreeWidget *getPcTreeWidget();
    /**
     * @brief Getter
     * @return ProviderWidget
     */
    ProviderWidget *getProviderWidget();
    QAction *getResetPasswdStorageAction();
    /**
     * @brief Getter
     * @return StatusBar
     */
    QStatusBar *getStatusBar();
    /**
     * @brief Getter
     * @return ToolBar
     */
    QToolBar *getToolbar();
    /**
     * @brief Close all dialogs
     * @param event -- not used
     */
    void closeEvent(QCloseEvent *event);

private slots:
    void closeAll();
    void showLog();
};

#endif // MAINWINDOW_H
