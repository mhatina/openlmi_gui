#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT

protected:
    void mousePressEvent(QMouseEvent *e);

public:
    TreeWidget(QWidget *parent = 0);

signals:
    void rightClick();
};

#endif // TREEWIDGET_H
