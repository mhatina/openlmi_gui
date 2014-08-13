#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>

class TreeWidgetItem : public QTreeWidgetItem
{
private:
    std::string m_ipv4;
    std::string m_ipv6;
    std::string m_name;
    std::string m_mac;

public:
    explicit TreeWidgetItem(QTreeWidgetItem *parent = 0);
    TreeWidgetItem(QTreeWidget *view);

    std::string getIpv4() const;
    void setIpv4(const std::string &value);

    std::string getIpv6() const;
    void setIpv6(const std::string &value);

    std::string getName() const;
    void setName(const std::string &value);

    std::string getMac() const;
    void setMac(const std::string &mac);
};

#endif // TREEWIDGETITEM_H
