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
#include "discoverworker.h"
#include "logger.h"
#include "pctreewidget.h"
#include "ui_pctreewidget.h"

#include <boost/thread.hpp>
#include <list>
#include <QRegExp>
#include <QTreeWidgetItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// public

PCTreeWidget::PCTreeWidget(QWidget *parent) :
    QWidget(parent),
    m_data_of_item_changed(false),
    m_worker(NULL),
    m_ui(new Ui::PCTreeWidget)
{
    m_ui->setupUi(this);
    loadPcs(SAVED_COMPUTER_PATH);    
    m_ui->tree->header()->resizeSection(0, 260);
    connect(
        m_ui->tree,
        SIGNAL(itemChanged(QTreeWidgetItem*,int)),
        this,
        SLOT(validate(QTreeWidgetItem*,int))
        );

    topLevelNode("Added")->setExpanded(true);
    setEditState(false);
}

PCTreeWidget::~PCTreeWidget()
{    
    saveAllPcs(SAVED_COMPUTER_PATH);
    delete m_ui;
    delete m_worker;
}

QTreeWidget* PCTreeWidget::getTree()
{
    return m_ui->tree;
}

QTreeWidgetItem* PCTreeWidget::topLevelNode(std::string item_name)
{
    QTreeWidgetItem *node;
    if (!(node = findTopLevelNode(item_name))) {
        node = new QTreeWidgetItem(m_ui->tree);
        node->setText(0, item_name.c_str());
        node->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
        m_ui->tree->sortByColumn(
                    0,
                    Qt::AscendingOrder
                    );
    }

    return node;
}

void PCTreeWidget::connectButtons(QToolBar *toolbar)
{
    QPushButton *button = toolbar->findChild<QPushButton*>("add_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onAddButtonClicked())
        );
    button = toolbar->findChild<QPushButton*>("remove_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onRemoveButtonClicked())
        );
    button = toolbar->findChild<QPushButton*>("discover_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onDiscoverButtonClicked())
        );
}

void PCTreeWidget::setEditState(bool state)
{
    Qt::ItemFlags flags_editable =
              Qt::ItemIsSelectable
            | Qt::ItemIsDragEnabled
            | Qt::ItemIsEnabled
            | Qt::ItemIsEditable;


    Qt::ItemFlags flags_non_editable =
              Qt::ItemIsSelectable
            | Qt::ItemIsEnabled;

    m_ui->tree->setSelectionMode(
                    state ? QAbstractItemView::ContiguousSelection : QAbstractItemView::SingleSelection);

    QTreeWidgetItem *parent = topLevelNode("Added");
    for (int i = 0; i < parent->childCount(); i++)
        parent->child(i)->setFlags(state ? flags_editable : flags_non_editable);
    if (m_ui->tree->topLevelItemCount() > 1) {
        parent = topLevelNode("Discovered");
        for (int i = 0; i < parent->childCount(); i++)
            parent->child(i)->setFlags(state ? flags_editable : flags_non_editable);
    }
}

// private

bool PCTreeWidget::parentContainsItem(QTreeWidgetItem *parent, std::string text)
{
    for (int i = 0; i < parent->childCount(); i++)
        if (parent->child(i)->text(0).toStdString() == text)
            return true;

    return false;
}

QTreeWidgetItem* PCTreeWidget::addPcToTree(std::string parent, std::string text)
{
    QTreeWidgetItem *parentItem = topLevelNode(parent);

    if (!parentContainsItem(parentItem, text)) {
        QTreeWidgetItem *child = new QTreeWidgetItem(parentItem);
        Qt::ItemFlags flags =
                  Qt::ItemIsSelectable
                | Qt::ItemIsDragEnabled
                | Qt::ItemIsEnabled
                | Qt::ItemIsEditable;
        m_data_of_item_changed = false;
        child->setFlags(flags);
        m_data_of_item_changed = true;

        if (text != "")
            child->setText(0, text.c_str());

        return child;
    }

    return NULL;
}

QTreeWidgetItem* PCTreeWidget::findTopLevelNode(std::string item_name)
{
    QTreeWidgetItem *tmp;
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        tmp = m_ui->tree->topLevelItem(i);
        if (tmp != NULL && tmp->text(0).toStdString() == item_name) {
            return tmp;
        }
    }

    return NULL;
}

void PCTreeWidget::loadPcs(std::string filename)
{
    QFile file(filename.c_str());
    if (!file.open( QIODevice::ReadOnly)) {
        Logger::getInstance()->error("Failed to read from " + filename + ", error: " + file.errorString().toStdString(), false);
        return;
    }

    QXmlStreamReader in(&file);
    if (in.hasError()) {
        Logger::getInstance()->error(in.errorString().toStdString(), false);
        return;
    }

    while (!in.atEnd()) {
        QXmlStreamReader::TokenType token = in.readNext();

        if (token == QXmlStreamReader::StartDocument)
            continue;
        else if(token == QXmlStreamReader::StartElement) {
            if (in.name() == "computers")
                continue;
            else if(in.name() == "computer") {
                QXmlStreamAttributes attr = in.attributes();
                addPcToTree("Added", attr.value("id").toString().toStdString());
            }
        }

    }
    if (in.hasError()) {
        Logger::getInstance()->error(in.errorString().toStdString(), false);
        return;
    }

    file.close();
}

void PCTreeWidget::saveAllPcs(std::string filename)
{
    QFile file(filename.c_str());
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::getInstance()->error("Failed to write to " + filename + ", error: " + file.errorString().toStdString(), false);
        return;
    }

    QXmlStreamWriter out(&file);
    QTreeWidgetItem* parent = topLevelNode("Added");

    out.setAutoFormatting(true);
    out.writeStartDocument();

    out.writeStartElement("computers");
    for (int i = 0; i < parent->childCount(); i++) {
        out.writeStartElement("computer");
        out.writeAttribute("id", parent->child(i)->text(0).toStdString().c_str());
        out.writeEndElement();
    }
    out.writeEndElement();

    out.writeEndDocument();
    file.close();
}

// slots

void PCTreeWidget::onAddButtonClicked()
{
    topLevelNode("Added")->setExpanded(true);

    std::list<QTreeWidgetItem*> tmp = m_ui->tree->selectedItems().toStdList();
    for (std::list<QTreeWidgetItem*>::iterator it = tmp.begin(); it != tmp.end(); it++)
        (*it)->setSelected(false);

    QTreeWidgetItem *child = addPcToTree("Added", "");
    if (child != NULL) {
        child->setSelected(true);
        m_ui->tree->editItem(child);
    }
}

void PCTreeWidget::onRemoveButtonClicked()
{
    QList<QTreeWidgetItem*> tmp = m_ui->tree->selectedItems();

    if (tmp.empty())
        return;

    for (QList<QTreeWidgetItem*>::Iterator it = tmp.begin(); it != tmp.end(); it++) {
        emit removed((*it)->text(0).toStdString());
        int pos = (*it)->parent()->indexOfChild(*it);
        (*it)->parent()->takeChild(pos);
    }
}

void PCTreeWidget::onDiscoverButtonClicked()
{    
    Logger::getInstance()->info("Discovering...");
    topLevelNode("Discovered");

    DiscoverWorker *m_worker = new DiscoverWorker();
    connect(
            m_worker,
            SIGNAL(finished(std::list<std::string>*)),
            this,
            SLOT(addDiscoveredPcsToTree(std::list<std::string>*))
            );
    boost::thread(boost::bind(&DiscoverWorker::discover, m_worker));
}

void PCTreeWidget::addDiscoveredPcsToTree(std::list<std::string> *pc)
{
    for (std::list<std::string>::iterator it = pc->begin(); it != pc->end(); it++) {
        addPcToTree("Discovered", (*it));
    }

    delete pc;
    delete m_worker;
    Logger::getInstance()->info("Finished discovering.");
}

void PCTreeWidget::validate(QTreeWidgetItem* item ,int column)
{
    if (!m_data_of_item_changed || item->text(0) == "Added"
            || item->text(0) == "Discovered")
        return;    
    QTreeWidgetItem *parent = item->parent();
    m_ui->tree->sortByColumn(0, Qt::AscendingOrder);

    if (item->text(column).isEmpty()) {
        parent->takeChild(parent->indexOfChild(item));
        return;
    }

    struct addrinfo *res;
    const char *ip = item->text(column).toStdString().c_str();
    int result;
    if (QRegExp(PATTERN_TYPE_FQDN).exactMatch(item->text(column)))
        result = 0;
    else {
        result = getaddrinfo(ip, NULL, NULL, &res);

        if (res->ai_family == AF_INET) {
            struct in_addr dstv4;
            result = inet_pton(res->ai_family, ip, &dstv4);
        } else if (res->ai_family == AF_INET6) {
            struct in6_addr dstv6;
            result = inet_pton(res->ai_family, ip, &dstv6);
        }
    }

    if (!result) {
        item->setBackground(0, QBrush(QColor("white")));
    } else {
        item->setSelected(false);
        item->setBackground(0, QBrush(QColor("red")));
    }
}

void PCTreeWidget::resendRemovedSignal(std::string id)
{
    emit removed(id);
}
