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

#ifndef PCTREEWIDGET_H
#define PCTREEWIDGET_H

#include "discoverworker.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolBar>
#include <QTreeWidgetItem>
#include <QWidget>

#ifndef PATTERN_TYPE_FQDN
#   define PATTERN_TYPE_FQDN \
        "(?=^.{1,254}$)(^(?:(?!\\d+\\.)[a-zA-Z0-9_\\-]{1,63}\\." \
            "?)+(?:[a-zA-Z]{2,})$)"
#endif

#ifndef PATTERN_TYPE_IPV4
#   define PATTERN_TYPE_IPV4 \
        "^(([\\d]{1,3})|(\\{([\\d]{1,3}|[\\d]{1,3}-[\\d]{1,3})(;(" \
            "[\\d]{1,3}|[\\d]{1,3}-[\\d]{1,3}))*\\}))\\.(([\\d]{1,3})" \
            "|(\\{([\\d]{1,3}|[\\d]{1,3}-[\\d]{1,3})(;([\\d]{1,3}|[\\d" \
            "]{1,3}-[\\d]{1,3}))*\\}))\\.(([\\d]{1,3})|(\\{([\\d]{1,3" \
            "}|[\\d]{1,3}-[\\d]{1,3})(;([\\d]{1,3}|[\\d]{1,3}-[\\d]{1" \
            ",3}))*\\}))\\.(([\\d]{1,3})|(\\{([\\d]{1,3}|[\\d]{1,3}-[" \
            "\\d]{1,3})(;([\\d]{1,3}|[\\d]{1,3}-[\\d]{1,3}))*\\}))$"
#endif

#ifndef PATTERN_TYPE_IPV6
#   define PATTERN_TYPE_IPV6 \
        "^(([\\da-fA-F]{1,4})|(\\{([\\da-fA-F]{1,4}|[\\da-fA-F]{" \
            "1,4}-[\\da-fA-F]{1,4})(;([\\da-fA-F]{1,4}|[\\da-fA-F]" \
            "{1,4}-[\\da-fA-F]{1,4}))*\\})){0,1}(:(([\\da-fA-F]{1," \
            "4})|(\\{([\\da-fA-F]{1,4}|[\\da-fA-F]{1,4}-[\\da-fA-F]"\
            "{1,4})(;([\\da-fA-F]{1,4}|[\\da-fA-F]{1,4}-[\\da-fA-F" \
            "]{1,4}))*\\})*)){0,6}:(([\\da-fA-F]{1,4})|(\\{([\\da-f" \
            "A-F]{1,4}|[\\da-fA-F]{1,4}-[\\da-fA-F]{1,4})(;([\\da-f" \
            "A-F]{1,4}|[\\da-fA-F]{1,4}-[\\da-fA-F]{1,4}))*\\}))$"
#endif

namespace Ui {
class PCTreeWidget;
}

/**
 * @brief The PCTreeWidget class
 *
 * Represent list of hosts.
 */
class PCTreeWidget : public QWidget
{
    Q_OBJECT

private:
    bool m_data_of_item_changed;
    DiscoverWorker *m_worker;
    Ui::PCTreeWidget *m_ui;

    bool parentContainsItem(QTreeWidgetItem *parent, std::string text);
    QTreeWidgetItem* addPcToTree(std::string parent, std::string text);
    QTreeWidgetItem* findTopLevelNode(std::string item_name);
    void loadPcs(std::string filename);
    void saveAllPcs(std::string filename);

private slots:
    void addDiscoveredPcsToTree(std::list<std::string> *pc);
    void onAddButtonClicked();
    void onDiscoverButtonClicked();
    void onRemoveButtonClicked();
    void resendRemovedSignal(std::string id);
    void validate(QTreeWidgetItem*, int column);

signals:
    /**
     * @brief Emitted when host is removed from list
     * @param id -- ip or domain name of host
     */
    void removed(std::string id);

public:    
    /**
     * @brief Constructor
     * @param parent -- parent of widget
     */
    explicit PCTreeWidget(QWidget *parent = 0);
    /**
     * @brief Destructor
     */
    ~PCTreeWidget();
    /**
     * @brief Getter
     * @return tree widget (list of hosts)
     */
    QTreeWidget* getTree();
    /**
     * @brief Find or create parent node of hosts
     * @param item_name -- parent node name
     * @return parent node
     */
    QTreeWidgetItem* topLevelNode(std::string item_name);
    /**
     * @brief Connect buttons' signals with handlers
     * @param toolbar -- contains buttons
     */
    void connectButtons(QToolBar *toolbar);
    /**
     * @brief Setter
     * @param state -- true if items can be edited, else false
     */
    void setEditState(bool state);
};

#endif // PCTREEWIDGET_H
