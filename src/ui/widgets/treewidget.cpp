#include "treewidget.h"

#include <iostream>
#include <QMouseEvent>

TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        emit rightClick();
        QTreeWidget::mousePressEvent(e);
    } else {
        QTreeWidget::mousePressEvent(e);
    }
}
