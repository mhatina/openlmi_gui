#include "treewidgetitem.h"

TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    m_ipv4(""),
    m_ipv6(""),
    m_name(""),
    m_mac("")
{
}

TreeWidgetItem::TreeWidgetItem(QTreeWidget *view) :
    QTreeWidgetItem(view)
{
}

std::string TreeWidgetItem::getIpv4() const
{
    return m_ipv4;
}

void TreeWidgetItem::setIpv4(const std::string &value)
{
    m_ipv4 = value;
}

std::string TreeWidgetItem::getIpv6() const
{
    return m_ipv6;
}

void TreeWidgetItem::setIpv6(const std::string &value)
{
    m_ipv6 = value;
}

std::string TreeWidgetItem::getName() const
{
    return m_name;
}

void TreeWidgetItem::setName(const std::string &value)
{
    m_name = value;
}

std::string TreeWidgetItem::getMac() const
{
    return m_mac;
}

void TreeWidgetItem::setMac(const std::string &mac)
{
    m_mac = mac;
}
