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
#include "kernel.h"
#include "logger.h"
#include "pctreewidget.h"
#include "systemdetailsdialog.h"
#include "treewidgetitem.h"
#include "ui_pctreewidget.h"

#include <boost/thread.hpp>
#include <list>
#include <QDir>
#include <QMenu>
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
    m_data_of_item_changed(false),
    m_emit_signal(true),
    m_new_item(false),
    m_worker(NULL),
    m_ui(new Ui::PCTreeWidget)
{
    Logger::getInstance()->debug("PCTreeWidget::PCTreeWidget(QWidget *parent)");
    m_ui->setupUi(this);
    std::string path = QDir::homePath().toStdString() + "/.config/lmicc_computers.xml";
    initContextMenu();

    connect(
        m_ui->tree,
        SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this,
        SLOT(validate(QTreeWidgetItem *, int))
    );
    connect(
        m_ui->tree,
        SIGNAL(itemEntered(QTreeWidgetItem *, int)),
        this,
        SLOT(startTime(QTreeWidgetItem *, int)));
    connect(
        &m_timer,
        SIGNAL(timeout()),
        this,
        SLOT(showSystemDetails()));
    connect(
        m_ui->tree,
        SIGNAL(customContextMenuRequested(QPoint)),
        this,
        SLOT(showContextMenu(QPoint)));
    connect(
        m_ui->tree,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(itemSelectionChanged()));
    connect(
        m_ui->tree,
        SIGNAL(rightClick()),
        this,
        SLOT(rightClicked()));
    connect(
        m_context_menu,
        SIGNAL(aboutToHide()),
        this,
        SLOT(menuHidden()));

    topLevelNode("Added")->setExpanded(true);
    loadPcs(path);
}

PCTreeWidget::~PCTreeWidget()
{
    Logger::getInstance()->debug("PCTreeWidget::~PCTreeWidget()");
    std::string path = QDir::homePath().toStdString() + "/.config/lmicc_computers.xml";
    saveAllPcs(path);
    if (m_ui != NULL) {
            delete m_ui;
            m_ui = NULL;
        }
    if (m_worker != NULL) {
            delete m_worker;
            m_worker = NULL;
        }
}

QTreeWidget *PCTreeWidget::getTree()
{
    Logger::getInstance()->debug("PCTreeWidget::getTree()");
    return m_ui->tree;
}

QTreeWidgetItem *PCTreeWidget::topLevelNode(std::string item_name)
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
    QPushButton *button = toolbar->findChild<QPushButton *>("add_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onAddButtonClicked())
    );
    button = toolbar->findChild<QPushButton *>("remove_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onRemoveButtonClicked())
    );
    button = toolbar->findChild<QPushButton *>("discover_button");
    connect(
        button,
        SIGNAL(clicked()),
        this,
        SLOT(onDiscoverButtonClicked())
    );
}

void PCTreeWidget::setComputerIcon(QIcon icon)
{
    Logger::getInstance()->debug("PCTreeWidget::setComputerIcon(QIcon icon)");
    TreeWidgetItem *item = (TreeWidgetItem *) m_ui->tree->selectedItems()[0];
    m_data_of_item_changed = false;
    item->setIcon(0, icon);
    m_data_of_item_changed = true;
}

// private


int PCTreeWidget::getTimeSec() const
{
    Logger::getInstance()->debug("PCTreeWidget::getTimeSec() const");
    return m_time_sec;
}

void PCTreeWidget::setTimeSec(int time_sec)
{
    Logger::getInstance()->debug("PCTreeWidget::setTimeSec(int time_sec)");
    m_time_sec = time_sec * 1000;
}

bool PCTreeWidget::parentContainsItem(QTreeWidgetItem *parent, std::string text, QTreeWidgetItem *item)
{
    Logger::getInstance()->debug("PCTreeWidget::parentContainsItem(TreeWidgetItem *parent, std::string text)");
    for (int i = 0; i < parent->childCount(); i++) {
        TreeWidgetItem *check_item = (TreeWidgetItem *) parent->child(i);
        if (item && check_item == item) {
            continue;
        }
        std::string ipv4 = check_item->getIpv4().empty() ? "N/A" : check_item->getIpv4();
        std::string ipv6 = check_item->getIpv6().empty() ? "N/A" : check_item->getIpv6();
        std::string domain = check_item->getName().empty() ? "N/A" : check_item->getName();
        std::string id = check_item->getId();
        if (ipv4 == text || ipv6 == text || domain == text || id == text) {
            return true;
        }
    }

    return false;
}

int PCTreeWidget::topLevelNodeCount(std::string item_name)
{
    Logger::getInstance()->debug("PCTreeWidget::topLevelNodeCount(std::string item_name)");
    int cnt = 0;
    TreeWidgetItem *tmp;
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        tmp = (TreeWidgetItem *) m_ui->tree->topLevelItem(i);
        if (item_name.empty()) {
            cnt++;
        } else if (tmp != NULL && tmp->text(0).toStdString() == item_name) {
            cnt++;
        }
    }

    return cnt;
}

std::string PCTreeWidget::getHostName(std::string &ip, int &ai_family)
{
    Logger::getInstance()->debug("PCTreeWidget::getHostName(std::string &ip, int &ai_family)");
    struct addrinfo hints;
    struct addrinfo *result;
    struct sockaddr_in sa4;
    struct sockaddr_in6 sa6;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    sa4.sin_family = AF_INET;
    sa6.sin6_family = AF_INET6;

    int err;
    if ((err = getaddrinfo(ip.c_str(), NULL, &hints, &result)) != 0) {
        switch (err) {
        case EAI_FAIL:
        case EAI_AGAIN:
            return "N/A";
        default:
            Logger::getInstance()->error(gai_strerror(err));
            return "";
        }
    }

    ai_family = result->ai_family;

    if ((err = inet_pton(result->ai_family, ip.c_str(),
                         ai_family == AF_INET ? (void *) &sa4.sin_addr : (void *) &sa6.sin6_addr)) != 1) {
        Logger::getInstance()->error(gai_strerror(err));
        return "";
    }

    struct hostent *h;
    if (ai_family == AF_INET) {
        h = gethostbyaddr((char *) &sa4.sin_addr, sizeof(struct in_addr), AF_INET);
    } else {
        h = gethostbyaddr((char *) &sa6.sin6_addr, sizeof(struct in6_addr), AF_INET6);
    }
    if (h == NULL) {
        return "N/A";
    }

    return h->h_name;
}

TreeWidgetItem *PCTreeWidget::addPcToTree(std::string parent, std::string text)
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
        child->setIcon(0, QIcon(":/computer.png"));

        if (text != "") {
            child->setText(0, text.c_str());
            child->setId(text);
        }
        m_data_of_item_changed = true;

        return child;
    }

    return NULL;
}

TreeWidgetItem *PCTreeWidget::findTopLevelNode(std::string item_name)
{
    Logger::getInstance()->debug("PCTreeWidget::findTopLevelNode(std::string item_name)");
    TreeWidgetItem *tmp;
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        tmp = (TreeWidgetItem *) m_ui->tree->topLevelItem(i);
        if (tmp != NULL && tmp->text(0).toStdString() == item_name) {
            return tmp;
        }
    }

    return NULL;
}

void PCTreeWidget::changeDisplayedName(TreeWidgetItem *item)
{
    if (!item->getName().empty()
        && item->getName() != "N/A"
        && (item->getName() != item->getIpv4()
            && item->getName() != item->getIpv6())) {
        std::string name = item->getName()
                           + " ("
                           + (item->getIpv4().empty() ? item->getIpv6() : item->getIpv4())
                           + ")";
        item->setText(0, name.c_str());
    }
}

void PCTreeWidget::getIp(std::string &name, std::string &ipv4,
                         std::string &ipv6)
{
    Logger::getInstance()->debug("PCTreeWidget::getIp(std::string &name, std::string &ipv4, std::string &ipv6)");
    struct addrinfo hints;
    struct addrinfo *result;
    void *ptr = NULL;
    char *dst = (char *) malloc(MAX_LENGTH * sizeof(char));

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if (getaddrinfo(name.c_str(), NULL, &hints, &result) != 0) {
        return;
    }

    while (result != NULL) {
        switch (result->ai_family) {
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

        if (result->ai_family == PF_INET6 && ipv6.empty()) {
            ipv6 = dst;
        } else if (result->ai_family == PF_INET && ipv4.empty()) {
            ipv4 = dst;
        }

        result = result->ai_next;
    }

    free(dst);
}

void PCTreeWidget::initContextMenu()
{
    Logger::getInstance()->debug("PCTreeWidget::initContextMenu()");
    m_context_menu = new QMenu(this);
    m_context_menu->setObjectName("pctree_context_menu");
    m_ui->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *action = m_context_menu->addAction("Refresh");
    action->setObjectName("refresh_action");
    action->setEnabled(false);

    m_context_menu->addSeparator();

    action = m_context_menu->addAction("Start LMIShell");
    action->setObjectName("start_lmishell_action");
    action->setEnabled(false);

    action = m_context_menu->addAction("Start ssh");
    action->setObjectName("start_ssh_action");
    action->setEnabled(false);

    m_context_menu->addSeparator();

    action = m_context_menu->addAction("Delete password");
    action->setObjectName("delete_passwd_action");
    action->setEnabled(false);

    action = m_context_menu->addAction("Delete system");
    action->setObjectName("delete_system_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(onRemoveButtonClicked()));
    action->setEnabled(false);
    m_context_menu->addSeparator();
    action = m_context_menu->addAction("Create new group");
    action->setObjectName("create_group_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(addGroup()));
    action = m_context_menu->addAction("Delete group");
    action->setObjectName("delete_group_action");
    connect(
        action,
        SIGNAL(triggered()),
        this,
        SLOT(deleteGroup()));
}

void PCTreeWidget::loadPcs(std::string filename)
{
    Logger::getInstance()->debug("PCTreeWidget::loadPcs(std::string filename)");
    QFile file(filename.c_str());
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        Logger::getInstance()->error("Failed to read from " + filename + ", error: " +
                                     file.errorString().toStdString(), false);
        return;
    }

    QXmlStreamReader in(&file);
    if (in.hasError()) {
        Logger::getInstance()->error(in.errorString().toStdString(), false);
        return;
    }

    QTreeWidgetItem *parent = NULL;
    while (!in.atEnd()) {
        QXmlStreamReader::TokenType token = in.readNext();

        if (token == QXmlStreamReader::StartDocument) {
            continue;
        } else {
            if (in.name() == "computers") {
                continue;
            } else if (in.name() == "group") {
                QXmlStreamAttributes attr = in.attributes();
                std::string name = attr.value("name").toString().toStdString();
                if (name.empty()) {
                    continue;
                }
                parent = topLevelNode(name);
            } else if (in.name() == "computer") {
                QXmlStreamAttributes attr = in.attributes();
                std::string id = attr.value("id").toString().toStdString();
                if (id.empty()) {
                    continue;
                }

                TreeWidgetItem *item = addPcToTree(parent->text(0).toStdString(), id);
                m_data_of_item_changed = false;
                if (item != NULL && parent != NULL) {
                    item->setIpv4(attr.value("ipv4").toString().toStdString());
                    item->setIpv6(attr.value("ipv6").toString().toStdString());
                    item->setName(attr.value("domain").toString().toStdString());
                    changeDisplayedName(item);
                    item->setMac(attr.value("mac").toString().toStdString());
                    if (attr.value("valid").toString() == "false") {
                        item->setValid(NOT_VALID);
                        item->setBackgroundColor(0, QColor("red"));
                    } else if (attr.value("valid").toString() == "unknown") {
                        item->setValid(UNKNOWN);
                        validate(item, 0);
                    } else {
                        item->setValid(VALID);
                    }
                }
                m_data_of_item_changed = true;
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
        Logger::getInstance()->error("Failed to write to " + filename + ", error: " +
                                     file.errorString().toStdString(), false);
        return;
    }

    QXmlStreamWriter out(&file);

    out.setAutoFormatting(true);
    out.writeStartDocument();

    out.writeStartElement("computers");
    for (int i = 0; i < m_ui->tree->topLevelItemCount(); i++) {
        QTreeWidgetItem *parent = m_ui->tree->topLevelItem(i);
        out.writeStartElement("group");
        out.writeAttribute("name", parent->text(0));
        for (int j = 0; j < parent->childCount(); j++) {
            TreeWidgetItem *item = (TreeWidgetItem *) parent->child(j);
            out.writeStartElement("computer");
            out.writeAttribute("id", item->getId().c_str());
            out.writeAttribute("ipv4", item->getIpv4().c_str());
            out.writeAttribute("ipv6", item->getIpv6().c_str());
            out.writeAttribute("domain", item->getName().c_str());
            out.writeAttribute("mac", item->getMac().c_str());

            std::string valid = "unknown";
            if (item->getValid() != UNKNOWN) {
                valid = item->getValid() == NOT_VALID ? "false" : "true";
            }
            out.writeAttribute("valid", valid.c_str());

            out.writeEndElement();
        }
        out.writeEndElement();
    }
    out.writeEndElement();

    out.writeEndDocument();
    file.close();
}

// slots

void PCTreeWidget::menuHidden()
{
    Logger::getInstance()->debug("PCTreeWidget::menuHidden()");
    m_emit_signal = true;
}

void PCTreeWidget::onAddButtonClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::onAddButtonClicked()");
    topLevelNode("Added")->setExpanded(true);

    m_emit_signal = false;

    std::list<QTreeWidgetItem *> tmp = m_ui->tree->selectedItems().toStdList();
    for (std::list<QTreeWidgetItem *>::iterator it = tmp.begin(); it != tmp.end();
         it++) {
        (*it)->setSelected(false);
    }

    TreeWidgetItem *child = addPcToTree("Added", "");
    if (child != NULL) {
        m_new_item = true;
        child->setSelected(true);
        m_ui->tree->setCurrentItem(child);
        m_ui->tree->editItem(child);
    }
}

void PCTreeWidget::onRemoveButtonClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::onRemoveButtonClicked()");
    QList<QTreeWidgetItem *> list = m_ui->tree->selectedItems();

    if (list.empty()) {
        return;
    }
    m_emit_signal = false;

    for (QList<QTreeWidgetItem *>::Iterator it = list.begin(); it != list.end();
         it++) {
        emit removed(((TreeWidgetItem *) (*it))->getId());
        int pos = (*it)->parent()->indexOfChild(*it);
        (*it)->parent()->takeChild(pos);
    }

    m_emit_signal = true;
}

void PCTreeWidget::onDiscoverButtonClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::onDiscoverButtonClicked()");
    emit refreshProgress(Engine::NOT_REFRESHED, "Discovering...");
    topLevelNode("Discovered");

    DiscoverWorker *m_worker = new DiscoverWorker();
    connect(
        m_worker,
        SIGNAL(finished(std::list<std::string> *)),
        this,
        SLOT(addDiscoveredPcsToTree(std::list<std::string> *))
    );
    boost::thread(boost::bind(&DiscoverWorker::discover, m_worker));
}

void PCTreeWidget::addDiscoveredPcsToTree(std::list<std::string> *pc)
{
    Logger::getInstance()->debug("PCTreeWidget::addDiscoveredPcsToTree(std::list<std::string> *pc)");
    for (std::list<std::string>::iterator it = pc->begin(); it != pc->end(); it++) {
        addPcToTree("Discovered", (*it));
    }

    if (pc != NULL) {
            delete pc;
            pc = NULL;
        }
    if (m_worker != NULL) {
            delete m_worker;
            m_worker = NULL;
        }
    emit refreshProgress(Engine::REFRESHED, "Discovering...");
}

void PCTreeWidget::addGroup()
{
    Logger::getInstance()->debug("PCTreeWidget::addGroup()");
    Qt::ItemFlags flags =
        Qt::ItemIsDropEnabled
        | Qt::ItemIsEnabled
        | Qt::ItemIsEditable;
    m_data_of_item_changed = false;
    QTreeWidgetItem *item = topLevelNode("");
    item->setFlags(flags);
    m_data_of_item_changed = true;
    m_ui->tree->editItem(item);
}

void PCTreeWidget::deleteGroup()
{
    Logger::getInstance()->debug("PCTreeWidget::deleteGroup()");
    QTreeWidgetItem *item = m_ui->tree->itemAt(m_item_pos);
    if (item == NULL || item->parent()) {
        return;
    }
    int index = m_ui->tree->indexOfTopLevelItem(item);
    m_ui->tree->takeTopLevelItem(index);
}

void PCTreeWidget::itemSelectionChanged()
{
    Logger::getInstance()->debug("PCTreeWidget::itemSelectionChanged()");
    if (m_emit_signal) {
        emit selectionChanged();
    }
}

void PCTreeWidget::rightClicked()
{
    Logger::getInstance()->debug("PCTreeWidget::rightClicked()");
    m_emit_signal = false;
}

void PCTreeWidget::showContextMenu(QPoint pos)
{
    Logger::getInstance()->debug("PCTreeWidget::showContextMenu(QPoint pos)");
    QPoint globalPos = m_ui->tree->mapToGlobal(pos);
    QTreeWidgetItem *item = m_ui->tree->itemAt(pos);
    m_item_pos = pos;

    if (item == NULL) {
        findChild<QAction *>("refresh_action")->setEnabled(false);
        findChild<QAction *>("start_lmishell_action")->setEnabled(false);
        findChild<QAction *>("start_ssh_action")->setEnabled(false);
        findChild<QAction *>("delete_passwd_action")->setEnabled(false);
        findChild<QAction *>("delete_system_action")->setEnabled(false);
        findChild<QAction *>("delete_group_action")->setEnabled(false);
    } else {
        bool enable = item->parent() != NULL;
        findChild<QAction *>("refresh_action")->setEnabled(enable);
        findChild<QAction *>("start_lmishell_action")->setEnabled(enable);
        findChild<QAction *>("start_ssh_action")->setEnabled(enable);
        findChild<QAction *>("delete_passwd_action")->setEnabled(enable);
        findChild<QAction *>("delete_system_action")->setEnabled(enable);
        findChild<QAction *>("delete_group_action")->setEnabled(!enable);
    }

    m_context_menu->popup(globalPos);
}

void PCTreeWidget::showSystemDetails()
{
    Logger::getInstance()->debug("PCTreeWidget::showSystemDetails()");
    QTreeWidgetItem *item = m_ui->tree->itemAt(
                                m_ui->tree->mapFromGlobal(
                                    QCursor::pos())
                            );
    if (item != m_item_to_show || m_context_menu->isVisible() ||
        !qApp->activeWindow()) {
        return;
    }
    m_dialog.reset();

    std::string tmp;
    if (!(tmp = m_item_to_show->getIpv4()).empty()) {
        m_dialog.setIpv4(tmp);
    }
    if (!(tmp = m_item_to_show->getIpv6()).empty()) {
        m_dialog.setIpv6(tmp);
    }
    if (!(tmp = m_item_to_show->getName()).empty()) {
        m_dialog.setDomain(tmp);
    }
    if (!(tmp = m_item_to_show->getMac()).empty()) {
        m_dialog.setMac(tmp);
    }


    m_dialog.move(QCursor::pos());
    m_dialog.show();
}

void PCTreeWidget::startTime(QTreeWidgetItem *item, int column)
{
    Logger::getInstance()->debug("PCTreeWidget::startTime(QTreeWidgetItem *item, int column)");
    Q_UNUSED(column)
    TreeWidgetItem *system = dynamic_cast<TreeWidgetItem *>(item);
    if (system == NULL) {
        return;
    }

    m_item_to_show = system;
    m_timer.start(m_time_sec);
}

void PCTreeWidget::validate(QTreeWidgetItem *item, int column)
{
    if (!m_data_of_item_changed) {
        return;
    }
    Logger::getInstance()->debug("PCTreeWidget::validate(TreeWidgetItem* item, int column)");
    TreeWidgetItem *parent = (TreeWidgetItem *) item->parent();
    if (parent == NULL) {
        if (item->text(0).size() == 0
            || topLevelNodeCount(item->text(0).toStdString()) > 1) {
            m_ui->tree->takeTopLevelItem(m_ui->tree->indexOfTopLevelItem(item));
        } else {

        }
        m_emit_signal = true;
        return;
    }

    TreeWidgetItem *tree_item = (TreeWidgetItem *) item;
    m_ui->tree->sortByColumn(0, Qt::AscendingOrder);

    m_data_of_item_changed = false;
    std::string text = tree_item->text(column).toStdString();
    if (text.empty() || parentContainsItem(parent, text, item)) {
        parent->takeChild(parent->indexOfChild(tree_item));
        m_emit_signal = true;
        return;
    }

    if (tree_item->getId().empty()) {
        tree_item->setId(text);
    }

    bool result = false;
    if (QRegExp(PATTERN_TYPE_FQDN).exactMatch(tree_item->getId().c_str())) {
        tree_item->setName(tree_item->getId());
        result = true;
        std::string ipv4, ipv6;
        getIp(text, ipv4, ipv6);
        if (!ipv4.empty()) {
            tree_item->setIpv4(ipv4);
        }

        if (!ipv6.empty()) {
            tree_item->setIpv6(ipv6);
        }
    } else {
        int ai_family;
        std::string ipv4, ipv6;
        std::string hostname = getHostName(text, ai_family);

        if (ai_family == AF_INET) {
            tree_item->setIpv4(ipv4 = tree_item->getId());
        } else if (ai_family == AF_INET6) {
            tree_item->setIpv6(ipv6 = tree_item->getId());
        }

        if ((result = !hostname.empty())) {
            // fetch missing ip (version 4 or 6)
            tree_item->setName(hostname);
            tree_item->setValid(hostname == "N/A" ? UNKNOWN : VALID);

            getIp(hostname, ipv4, ipv6);
            if (!ipv4.empty()) {
                tree_item->setIpv4(ipv4);
            }

            if (!ipv6.empty()) {
                tree_item->setIpv6(ipv6);
            }
        }
    }

    m_emit_signal = true;
    if (result) {
        tree_item->setBackground(0, QBrush(QColor("white")));

        Qt::ItemFlags flags =
            Qt::ItemIsSelectable
            | Qt::ItemIsDragEnabled
            | Qt::ItemIsEnabled;

        tree_item->setFlags(flags);
        changeDisplayedName(tree_item);

        if (m_new_item) {
            // little hack for autorefresh
            tree_item->setSelected(false);
            tree_item->setSelected(true);
        }
    } else {
        tree_item->setSelected(false);
        tree_item->setBackground(0, QBrush(QColor("red")));
        tree_item->setValid(NOT_VALID);
    }
    m_data_of_item_changed = true;
    m_new_item = false;
}
