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

#include "logicalfile.h"
#include "cimvalue.h"
#include "ui_logicalfile.h"

#include <sstream>

bool sortInstances(void *first, void *second)
{
    Pegasus::CIMInstance first_instance = *((Pegasus::CIMInstance *) first);
    Pegasus::CIMInstance second_instance = *((Pegasus::CIMInstance *) second);
    if (CIMValue::get_property_value(first_instance, "CreationClassName") == "LMI_UnixDirectory"
        && CIMValue::get_property_value(second_instance, "CreationClassName") == "LMI_DataFile") {
        return true;
    } else if (CIMValue::get_property_value(first_instance, "CreationClassName") == "LMI_SymbolicLink"
               && CIMValue::get_property_value(second_instance, "CreationClassName") == "LMI_DataFile") {
        return true;
    } else if ((CIMValue::get_property_value(first_instance, "CreationClassName") == "LMI_UnixDirectory"
                && CIMValue::get_property_value(second_instance, "CreationClassName") == "LMI_SymbolicLink")
               ||
               (CIMValue::get_property_value(first_instance, "CreationClassName") == "LMI_SymbolicLink"
                && CIMValue::get_property_value(second_instance, "CreationClassName") == "LMI_UnixDirectory")
               ||
               (CIMValue::get_property_value(first_instance, "CreationClassName") ==
                CIMValue::get_property_value(second_instance, "CreationClassName"))) {
        return CIMValue::get_property_value(first_instance, "Name") < CIMValue::get_property_value(second_instance, "Name");
    }

    return false;
}

FileTree::Item *LogicalFilePlugin::emitFound(std::vector<void *> *data, FileTree::Item *parent)
{
    FileTree::Item *found;
    if ((found = m_tree.findItem(parent->getFullName()))->count() != 0) {
        for (unsigned int i = 0; i < found->count(); i++) {
            data->push_back(new Pegasus::CIMInstance(found->getDescendant(i).getCIMData()));
        }
        ((QGroupBox *) getSelectedTree()->parent())->setTitle(found->getFullName());
        emit writeData(data, found);
    }

    return found;
}

void LogicalFilePlugin::getInfo(std::vector<void *> *data, FileTree::Item *parent)
{
    emit refreshProgress(Engine::NOT_REFRESHED, this);

    QTreeWidget *current = getSelectedTree();

    FileTree::Item *found;
    if (parent->text(0) == "..") {
        parent = parent->getParent();
    }

    if ((found = emitFound(data, parent))->count() != 0) {
        return;
    }

    ((QGroupBox *) current->parent())->setTitle(parent->getFullName());

    try {
        Pegasus::Array<Pegasus::CIMObject> objects =
            associators(
                Pegasus::CIMNamespaceName("root/cimv2"),
                found->getCIMData().getPath());

        for (unsigned int i = 0; i < objects.size(); i++) {
            data->push_back(new Pegasus::CIMInstance(objects[i]));
        }

        std::sort(data->begin(), data->end(), sortInstances);
    } catch (Pegasus::Exception &ex) {
        delete data;
        emit refreshProgress(Engine::REFRESHED, this);
        Logger::getInstance()->error(CIMValue::to_string(ex.getMessage()));
        return;
    }

    emit writeData(data, found);
}

void LogicalFilePlugin::getSymlinkInfo(std::vector<void *> *data, FileTree::Item *parent)
{
    Logger::getInstance()->info("Not possible to expand symlink");
    return;

    if (parent->text(0) == "..") {
        getInfo(data, parent->getParent());
        return;
    }

    emit refreshProgress(Engine::NOT_REFRESHED, this);

    FileTree::Item *found;
    QTreeWidget *current = getSelectedTree();
    if ((found = emitFound(data, parent))->count() != 0) {
        return;
    }

    ((QGroupBox *) current->parent())->setTitle(parent->getFullName());

    try {
        Pegasus::CIMInstance instance = found->getCIMData();
        Pegasus::Array<Pegasus::CIMKeyBinding> bindings;
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("FSCreationClassName"),
                            CIMValue::get_property_value(instance, "FSCreationClassName"),
                            Pegasus::CIMKeyBinding::STRING));
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("Name"),
                            CIMValue::get_property_value(instance, "TargetFile"),
                            Pegasus::CIMKeyBinding::STRING));
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("CSName"),
                            CIMValue::get_property_value(instance, "CSName"),
                            Pegasus::CIMKeyBinding::STRING));
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("CSCreationClassName"),
                            CIMValue::get_property_value(instance, "CSCreationClassName"),
                            Pegasus::CIMKeyBinding::STRING));
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("CreationClassName"),
                            Pegasus::String("LMI_UnixDirectory"),
                            Pegasus::CIMKeyBinding::STRING));
        bindings.append(Pegasus::CIMKeyBinding(
                            Pegasus::CIMName("FSName"),
                            CIMValue::get_property_value(instance, "FSName"),
                            Pegasus::CIMKeyBinding::STRING));

        Pegasus::CIMObjectPath path;
        path.set(
            m_client->hostname(),
            Pegasus::CIMNamespaceName("root/cimv2"),
            Pegasus::CIMName("LMI_UnixDirectory"),
            bindings
        );

        Pegasus::Array<Pegasus::CIMObject> objects =
            associators(
                Pegasus::CIMNamespaceName("root/cimv2"),
                path);

        for (unsigned int i = 0; i < objects.size(); i++) {
            data->push_back(new Pegasus::CIMInstance(objects[i]));
        }

        std::sort(data->begin(), data->end(), sortInstances);
    } catch (Pegasus::Exception &ex) {
        delete data;
        emit refreshProgress(Engine::REFRESHED, this);
        Logger::getInstance()->error(CIMValue::to_string(ex.getMessage()));
        return;
    }

    emit writeData(data, found);
}

QTreeWidget *LogicalFilePlugin::getNotSelectedTree(QTreeWidgetItem *item)
{
    if (m_ui->left_list->indexOfTopLevelItem(item) != -1) {
        return m_ui->right_list;
    } else if (m_ui->right_list->indexOfTopLevelItem(item) != -1) {
        return m_ui->left_list;
    } else {
        return NULL;
    }
}

QTreeWidget *LogicalFilePlugin::getSelectedTree()
{
    if (!m_ui->left_list->selectedItems().empty()) {
        return m_ui->left_list;
    } else if (!m_ui->right_list->selectedItems().empty()) {
        return m_ui->right_list;
    } else {
        return NULL;
    }
}

void LogicalFilePlugin::populateTree(QTreeWidget *tree, std::vector<void *> *data, FileTree::Item *parent)
{
    unsigned int cnt = data->size();
    try {
        bool parent_is_link = CIMValue::get_property_value(parent->getCIMData(), "CreationClassName") == "LMI_SymbolicLink";
        for (unsigned int i = 0; i < cnt; i++) {
            bool file = false;
            bool link = false;
            Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[i]);

            if (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_UnixDirectory"
                || (file = (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_DataFile"))
                || (link = (CIMValue::get_property_value(*instance, "CreationClassName") == "LMI_SymbolicLink"))) {
                FileTree::Item *item = link
                                       ? new FileTree::SymbolicLink(parent)
                                       : new FileTree::Item(parent);
                item->setCIMData(instance);

                item->setText(1, CIMValue::get_property_value(*instance, "FileSize"));

                String rights = CIMValue::get_property_value(*instance, "Readable") == "True" ? "r" : "-";
                rights += CIMValue::get_property_value(*instance, "Writeable") == "True" ? "w" : "-";
                rights += CIMValue::get_property_value(*instance, "Executable") == "True" ? "x" : "-";
                item->setText(2, rights);

                item->setText(3, CIMValue::get_property_value(*instance, "LastModified"));
                item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

                String name = CIMValue::get_property_value(*instance, "Name");
                unsigned int length = parent_is_link
                                      ? ((FileTree::SymbolicLink *) parent)->getTargetName().length()
                                      : parent->getFullName().length();

                if (name.length() <= length) {
                    data->erase(data->begin() + i);
                    i--;
                    cnt--;
                    delete instance;
                    continue;
                }

                if (length != 1) {
                    name = name.substr(length);
                }

                if (file) {
                    name = name.substr(1);
                } else if (link) {
                    name[0] = '~';
                }

                item->setText(0, name);

                tree->addTopLevelItem(item);

                FileTree::Item copy(*item);
                parent->addDescendant(copy);
            } else {
                data->erase(data->begin() + i);
                i--;
                cnt--;
                delete instance;
            }
        }
    } catch (Pegasus::Exception &ex) {
        Logger::getInstance()->error(ex.getMessage());
        return;
    }
}

LogicalFilePlugin::LogicalFilePlugin() :
    IPlugin(),
    m_file_cnt(0),
    m_ui(new Ui::LogicalFilePlugin)
{
    m_ui->setupUi(this);
    setPluginEnabled(false);

    m_ui->filter_box->hide();
    m_ui->left_list->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->right_list->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(
        m_ui->left_list,
        SIGNAL(enterDir(QTreeWidgetItem *)),
        this,
        SLOT(dirEntered(QTreeWidgetItem *)));
    connect(
        m_ui->right_list,
        SIGNAL(enterDir(QTreeWidgetItem *)),
        this,
        SLOT(dirEntered(QTreeWidgetItem *)));
    connect(
        m_ui->left_list,
        SIGNAL(enterSymlink(QTreeWidgetItem *)),
        this,
        SLOT(symlinkEntered(QTreeWidgetItem *)));
    connect(
        m_ui->right_list,
        SIGNAL(enterSymlink(QTreeWidgetItem *)),
        this,
        SLOT(symlinkEntered(QTreeWidgetItem *)));
    connect(
        this,
        SIGNAL(writeData(std::vector<void *> *, FileTree::Item *)),
        this,
        SLOT(writeInfo(std::vector<void *> *, FileTree::Item *)));
    connect(
        m_ui->left_list,
        SIGNAL(itemClicked(QTreeWidgetItem *, int)),
        this,
        SLOT(changeCursor(QTreeWidgetItem *, int)));
    connect(
        m_ui->right_list,
        SIGNAL(itemClicked(QTreeWidgetItem *, int)),
        this,
        SLOT(changeCursor(QTreeWidgetItem *, int)));
}

LogicalFilePlugin::~LogicalFilePlugin()
{
    delete m_ui;
}

String LogicalFilePlugin::getInstructionText()
{
    std::stringstream ss;
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        ss << m_instructions[i]->toString();
    }
    return ss.str();
}

String LogicalFilePlugin::getLabel()
{
    return "File browser";
}

String LogicalFilePlugin::getRefreshInfo()
{
    std::stringstream ss;
    ss << "Nothing to show";

    return ss.str();
}

void LogicalFilePlugin::clear()
{
    m_ui->left_list->clear();
    m_ui->right_list->clear();
}

void LogicalFilePlugin::getData(std::vector<void *> *data)
{
    try {
        Pegasus::Array<Pegasus::CIMInstance> root =
            enumerateInstances(
                Pegasus::CIMNamespaceName("root/cimv2"),
                Pegasus::CIMName("LMI_RootDirectory"),
                true,       // deep inheritance
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );

        if (root.size() != 1) {
            emit doneFetchingData(NULL, false, "No root directory");
            return;
        }

        data->push_back(new Pegasus::CIMInstance(root[0]));

        Pegasus::Uint32 ind = root[0].findProperty("PartComponent");
        Pegasus::CIMProperty property = root[0].getProperty(ind);
        Pegasus::CIMValue value = property.getValue();

        Pegasus::CIMObjectPath path;
        value.get(path);
        Pegasus::CIMInstance root_instance =
            m_client->getInstance(
                Pegasus::CIMNamespaceName("root/cimv2"),
                path,
                false,      // local only
                false,      // include qualifiers
                false       // include class origin
            );
        root_instance.setPath(path);

        Pegasus::Array<Pegasus::CIMObject> assoc =
            associators(
                Pegasus::CIMNamespaceName("root/cimv2"),
                root_instance.getPath()
            );

        for (unsigned int i = 0; i < assoc.size(); i++) {
            if (CIMValue::get_property_value(Pegasus::CIMInstance(assoc[i]), "Name") == "/") {
                continue;
            }
            data->push_back(new Pegasus::CIMInstance(assoc[i]));
        }

        std::sort(data->begin() + 1, data->end(), sortInstances);
    } catch (Pegasus::Exception &ex) {
        emit doneFetchingData(NULL, false, ex.getMessage());
        return;
    }

    emit doneFetchingData(data);
}

void LogicalFilePlugin::fillTab(std::vector<void *> *data)
{
    Pegasus::CIMInstance *instance = ((Pegasus::CIMInstance *) (*data)[0]);

    Pegasus::Uint32 ind = instance->findProperty("PartComponent");
    if (ind == Pegasus::PEG_NOT_FOUND) {
        return;
    }

    FileTree::RootItem root_item(m_client);
    root_item.setCIMData(instance);
    m_tree.setRoot(root_item);

    data->erase(data->begin());

    ((QGroupBox *) m_ui->left_list->parent())->setTitle(root_item.getFullName());
    populateTree(m_ui->left_list, data, &m_tree.getRoot());
    ((QGroupBox *) m_ui->right_list->parent())->setTitle(root_item.getFullName());
    populateTree(m_ui->right_list, data, &m_tree.getRoot());
}

void LogicalFilePlugin::changeCursor(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    QTreeWidget *other = getNotSelectedTree(item);

    if (!other->selectedItems().empty()) {
        other->selectedItems()[0]->setSelected(false);
    }
}

void LogicalFilePlugin::dirEntered(QTreeWidgetItem *item)
{
    FileTree::Item *file = (FileTree::Item *) item;
    std::vector<void *> *data = new std::vector<void *>();
    boost::thread(boost::bind(&LogicalFilePlugin::getInfo, this, data, file));
}

void LogicalFilePlugin::symlinkEntered(QTreeWidgetItem *item)
{
    FileTree::Item *file = (FileTree::Item *) item;
    std::vector<void *> *data = new std::vector<void *>();
    boost::thread(boost::bind(&LogicalFilePlugin::getSymlinkInfo, this, data, file));
}

void LogicalFilePlugin::writeInfo(std::vector<void *> *data, FileTree::Item *parent)
{
    QTreeWidget *tree = getSelectedTree();

    tree->clear();

    populateTree(tree, data, parent);
    if ((*parent) != m_tree.getRoot()) {
        FileTree::Item *copy_parent = new FileTree::Item(*parent);
        tree->insertTopLevelItem(0, copy_parent);
        copy_parent->setText(0, "..");
        copy_parent->setSelected(true);
    }

    delete data;

    emit refreshProgress(Engine::REFRESHED, this);
}

Q_EXPORT_PLUGIN2(logicalFile, LogicalFilePlugin)
