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

#include "discoverworker.h"
#include "logger.h"
#include "pctreewidget.h"
#include "systemdetailsdialog.h"
#include "treewidgetitem.h"
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
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_LENGTH 256

// public

PCTreeWidget::PCTreeWidget(QWidget *parent) :
    QWidget(parent),
    m_change_text(true),
    m_data_of_item_changed(false),
    m_worker(NULL),
    m_ui(new Ui::PCTreeWidget)
{
    Logger::getInstance()->debug("PCTreeWidget::PCTreeWidget(QWidget *parent)");
    m_ui->setupUi(this);
    std::string path = "/home/mhatina/.openlmi/openlmi_computers.xml";    
    m_ui->tree->header()->resizeSection(0, 260);

    connect(
        m_ui->tree,
        SIGNAL(itemChanged(QTreeWidgetItem*,int)),
        this,
        SLOT(validate(QTreeWidgetItem*,int))
        );
    connect(
        m_ui->tree,
        SIGNAL(itemEntered(QTreeWidgetItem*,int)),
        this,
        SLOT(startTime(QTreeWidgetItem*,int)));
    connect(
        &m_timer,
        SIGNAL(timeout()),
        this,
        SLOT(showSystemDetails()));

    topLevelNode("Added")->setExpanded(true);
    loadPcs(path);
    setEditState(false);

    m_dialog.setWindowFlags(Qt::Popup);
}

PCTreeWidget::~PCTreeWidget()
{
    Logger::getInstance()->debug("PCTreeWidget::~PCTreeWidget()");
    m_change_text = false;
    std::string path = "/home/mhatina/.openlmi/openlmi_computers.xml";
    saveAllPcs(path);
    delete m_ui;
    delete m_worker;
}

QTreeWidget* PCTreeWidget::getTree()
{
    Logger::getInstance()->debug("PCTreeWidget::getTree()");
    return m_ui->tree;
}

QTreeWidgetItem* PCTreeWidget::topLevelNode(std::string item_name)
{
    Logger::getInstance()->debug("PCTreeWidget::topLevelNode(std::string item_name)");
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
    Logger::getInstance()->debug("PCTreeWidget::connectButtons(QToolBar *toolbar)");
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
    Logger::getInstance()->debug("PCTreeWidget::setEditState(bool state)");
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

    m_data_of_item_changed = false;
    QTreeWidgetItem *parent = topLevelNode("Added");
    for (int i = 0; i < parent->childCount(); i++)
        parent->child(i)->setFlags(state ? flags_editable : flags_non_editable);
    if (m_ui->tree->topLevelItemCount() > 1) {
        parent = topLevelNode("Discovered");
        for (int i = 0; i < parent->childCount(); i++)
            parent->child(i)->setFlags(state ? flags_editable : flags_non_editable);
    }
    m_data_of_item_changed = true;
}

void PCTreeWidget::setComputerIcon(QIcon icon)
{
    Logger::getInstance()->debug("PCTreeWidget::setComputerIcon(QIcon icon)");
    TreeWidgetItem *item = (TreeWidgetItem*) m_ui->tree->selectedItems()[0];
    m_data_of_item_changed = false;
//    item->setToolTip(0, "");
    item->setIcon(0, icon);
    m_data_of_item_changed = true;
}

// private


int PCTreeWidget::getTimeSec() const
{
    return m_time_sec;
}

void PCTreeWidget::setTimeSec(int time_sec)
{
    m_time_sec = time_sec * 1000;
}

bool PCTreeWidget::parentContainsItem(QTreeWidgetItem *parent, std::string text)
{
    Logger::getInstance()->debug("PCTreeWidget::parentContainsItem(TreeWidgetItem *parent, std::string text)");
    for (int i = 0; i < parent->childCount(); i++)
        if (parent->child(i)->text(0).toStdString() == text)
            return true;

    return false;
}

TreeWidgetItem* PCTreeWidget::addPcToTree(std::string parent, std::string text)
{
    Logger::getInstance()->debug("PCTreeWidget::addPcToTree(std::string parent, std::string text)");

    QTreeWidgetItem *parentItem = topLevelNode(parent);

    if (!parentContainsItem(parentItem, text)) {
        TreeWidgetItem *child = new TreeWidgetItem(parentItem);
        Qt::ItemFlags flags =
                  Qt::ItemIsSelectable
                | Qt::ItemIsDragEnabled
                | Qt::ItemIsEnabled
                | Qt::ItemIsEditable;
        m_data_of_item_changed = false;
        child->setFlags(flags);
//        child->setToolTip(0, "Unknown");
        child->setIcon(0, QIcon(":/computer.png"));
        m_data_of_item_changed = true;

        if (text != "") {
            child->setText(0, text.c_str());
        }

        return child;
    }

    return NULL;
}

TreeWidgetItem* PCTreeWidget::findTopLevelNode(std::string item_name)
{
    Logger::getInstance()->debug("PCTreeWidget::findTopLevelNode(std::string item_name)");
    TreeWidgetItem *tmp;
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        tmp = (TreeWidgetItem*) m_ui->tree->topLevelItem(i);
        if (tmp != NULL && tmp->text(0).toStdString() == item_name) {
            return tmp;
        }
    }

    return NULL;
}

void PCTreeWidget::loadPcs(std::string filename)
{
    Logger::getInstance()->debug("PCTreeWidget::loadPcs(std::string filename)");
    QFile file(filename.c_str());
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly)) {
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
        else {
            if (in.name() == "computers")
                continue;
            else if(in.name() == "computer") {
                QXmlStreamAttributes attr = in.attributes();
                std::string id = attr.value("id").toString().toStdString();
                if (id.empty())
                    continue;

                TreeWidgetItem *item = addPcToTree("Added", id);
                if (item != NULL) {
                    item->setIpv4(attr.value("ipv4").toString().toStdString());
                    item->setIpv6(attr.value("ipv6").toString().toStdString());
                    item->setName(attr.value("domain").toString().toStdString());
                    item->setMac(attr.value("mac").toString().toStdString());
                }
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
    Logger::getInstance()->debug("PCTreeWidget::saveAllPcs(std::string filename)");
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
        out.writeAttribute("id", parent->child(i)->text(0));
        out.writeAttribute("ipv4", ((TreeWidgetItem*) parent->child(i))->getIpv4().c_str());
        out.writeAttribute("ipv6", ((TreeWidgetItem*) parent->child(i))->getIpv6().c_str());
        out.writeAttribute("domain", ((TreeWidgetItem*) parent->child(i))->getName().c_str());
        out.writeAttribute("mac", ((TreeWidgetItem*) parent->child(i))->getMac().c_str());
        out.writeEndElement();
    }
    out.writeEndElement();

    out.writeEndDocument();
    file.close();
}

// slots

void PCTreeWidget::onAddButtonClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::onAddButtonClicked()");
    topLevelNode("Added")->setExpanded(true);

    std::list<QTreeWidgetItem*> tmp = m_ui->tree->selectedItems().toStdList();
    for (std::list<QTreeWidgetItem*>::iterator it = tmp.begin(); it != tmp.end(); it++)
        (*it)->setSelected(false);

    TreeWidgetItem *child = addPcToTree("Added", "");
    if (child != NULL) {
        child->setSelected(true);
        m_ui->tree->editItem(child);
    }
}

void PCTreeWidget::onRemoveButtonClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::onRemoveButtonClicked()");
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
    Logger::getInstance()->debug("PCTreeWidget::onDiscoverButtonClicked()");
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
    Logger::getInstance()->debug("PCTreeWidget::addDiscoveredPcsToTree(std::list<std::string> *pc)");
    for (std::list<std::string>::iterator it = pc->begin(); it != pc->end(); it++) {
        addPcToTree("Discovered", (*it));
    }

    delete pc;
    delete m_worker;
    Logger::getInstance()->info("Finished discovering.");
}

void PCTreeWidget::showSystemDetails()
{
    QTreeWidgetItem *item = m_ui->tree->itemAt(m_ui->tree->mapFromGlobal(QCursor::pos()));
    if (item != m_item_to_show)
        return;
    m_dialog.reset();

    std::string tmp;
    if (!(tmp = m_item_to_show->getIpv4()).empty())
        m_dialog.setIpv4(tmp);
    if (!(tmp = m_item_to_show->getIpv6()).empty())
        m_dialog.setIpv6(tmp);
    if (!(tmp = m_item_to_show->getName()).empty())
        m_dialog.setDomain(tmp);
    if (!(tmp = m_item_to_show->getMac()).empty())
        m_dialog.setMac(tmp);


    m_dialog.move(QCursor::pos());
    m_dialog.show();
}

void PCTreeWidget::startTime(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    TreeWidgetItem *system = dynamic_cast<TreeWidgetItem*>(item);
    if (system == NULL)
        return;

    m_item_to_show = system;
    m_timer.start(m_time_sec);
}

void PCTreeWidget::validate(QTreeWidgetItem* item ,int column)
{
    Logger::getInstance()->debug("PCTreeWidget::validate(TreeWidgetItem* item ,int column)");
    if (!m_data_of_item_changed || item->text(0) == "Added"
            || item->text(0) == "Discovered")
        return;    
    TreeWidgetItem *parent = (TreeWidgetItem*) item->parent();
    if (parent == NULL)
        return;
    m_ui->tree->sortByColumn(0, Qt::AscendingOrder);

    if (item->text(column).isEmpty()) {
        parent->takeChild(parent->indexOfChild(item));
        return;
    }

    std::string str_ip = item->text(column).toStdString();
    char ip[MAX_LENGTH];
    strncpy(ip, str_ip.c_str(), MAX_LENGTH - 1);
    int result;
    if (QRegExp(PATTERN_TYPE_FQDN).exactMatch(item->text(column))) {
        ((TreeWidgetItem*) item)->setName(item->text(0).toStdString());
        result = 0;
        std::string ipv4, ipv6;
        getIp(str_ip, ipv4, ipv6);
        if (!ipv4.empty())
            ((TreeWidgetItem*) item)->setIpv4(ipv4);

        if (!ipv6.empty())
            ((TreeWidgetItem*) item)->setIpv6(ipv6);
    } else {
        int ai_family;
        std::string ipv4, ipv6;
        std::string hostname = getHostName(str_ip, ai_family);

        if (ai_family == AF_INET) {
            ((TreeWidgetItem*) item)->setIpv4(ipv4 = item->text(0).toStdString());
        } else if (ai_family == AF_INET6) {
            ((TreeWidgetItem*) item)->setIpv6(ipv6 = item->text(0).toStdString());
        }

        if (hostname.empty())
            result = -1;
        else {
            // fetch missing ip (version 4 or 6)
            ((TreeWidgetItem*) item)->setName(hostname);

            getIp(hostname, ipv4, ipv6);
            if (!ipv4.empty())
                ((TreeWidgetItem*) item)->setIpv4(ipv4);

            if (!ipv6.empty())
                ((TreeWidgetItem*) item)->setIpv6(ipv6);
        }
    }

    if (!result) {
        item->setBackground(0, QBrush(QColor("white")));
    } else {
        item->setSelected(false);
        item->setBackground(0, QBrush(QColor("red")));
    }
}

std::string PCTreeWidget::getHostName(std::string &ip, int &ai_family)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct sockaddr_in sa4;
    struct sockaddr_in6 sa6;
    char name[NI_MAXHOST];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    sa4.sin_family = AF_INET;
    sa6.sin6_family = AF_INET6;

    if (getaddrinfo(ip.c_str(), NULL, &hints, &result) != 0) {
        return "";
    }

    ai_family = result->ai_family;

    if (inet_pton(result->ai_family, ip.c_str(), ai_family == AF_INET ? (void*) &sa4.sin_addr : (void*) &sa6.sin6_addr) != 1)
        return "";
    int err;
    struct sockaddr *sa = (ai_family == AF_INET ? (struct sockaddr*) &sa4 : (struct sockaddr*) &sa6);

    // BUG not working for ipv6
    if ((err = getnameinfo(sa, sizeof(*sa), name, sizeof(name), NULL, 0, 0))) {
        Logger::getInstance()->error(gai_strerror(err));
        return "";
    }

    return name;
}

void PCTreeWidget::getIp(std::string &name, std::string &ipv4, std::string &ipv6)
{
    struct addrinfo hints;
    struct addrinfo *result;
    void *ptr;
    char *dst = (char*) malloc(MAX_LENGTH * sizeof(char));

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if (getaddrinfo(name.c_str(), NULL, &hints, &result) != 0) {
        return;
    }

    while (result != NULL)
    {
        switch(result->ai_family)
        {
        case AF_INET:
            ptr = &((struct sockaddr_in *) result->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) result->ai_addr)->sin6_addr;
            break;
        default:
            break;
        }

        inet_ntop(result->ai_family, ptr, dst, MAX_LENGTH);

        if (result->ai_family == PF_INET6 && ipv6.empty())
            ipv6 = dst;
        else if (ipv4.empty())
            ipv4 = dst;

        result = result->ai_next;
    }

    free(dst);
}
