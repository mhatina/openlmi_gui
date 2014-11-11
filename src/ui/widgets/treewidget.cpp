#include "treewidget.h"

#include <iostream>
#include <QMouseEvent>

TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidget::dropEvent(event);

    if (m_dragged_item && !m_dragged_item->parent()) {
        takeTopLevelItem(indexOfTopLevelItem(m_dragged_item));
        m_old_parent->addChild(m_dragged_item);
        return;
    }
}

void TreeWidget::mousePressEvent(QMouseEvent *e)
{
    m_dragged_item = itemAt(e->pos());
    if (m_dragged_item) {
        m_old_parent = m_dragged_item->parent();
    }

    if (e->button() == Qt::RightButton) {
        emit rightClick();
        QTreeWidget::mousePressEvent(e);
    } else {
        QTreeWidget::mousePressEvent(e);
    }
}
