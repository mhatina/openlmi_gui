#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT

private:
    QTreeWidgetItem *m_dragged_item;
    QTreeWidgetItem *m_old_parent;

protected:
    void dropEvent(QDropEvent *event);
    void mousePressEvent(QMouseEvent *e);

public:
    TreeWidget(QWidget *parent = 0);

signals:
    void rightClick();
};

#endif // TREEWIDGET_H
