#include "listwidget.h"
#include "ui_listwidget.h"

ListWidget::ListWidget(QWidget *parent) :
    QListWidget(parent),
    m_ui(new Ui::ListWidget)
{
    m_ui->setupUi(this);
}

ListWidget::~ListWidget()
{
    delete m_ui;
}

QList<QListWidgetItem*> ListWidget::selectedItems() const
{
    return m_ui->listWidget->selectedItems();
}

void ListWidget::addItem(QListWidgetItem *item)
{
    m_ui->listWidget->addItem(item);
}

void ListWidget::removeItem(QListWidgetItem *item)
{
    m_ui->listWidget->removeItemWidget(item);
}

#include <iostream>

void ListWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    emit getFocus();
}

void ListWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    emit lostFocus();
}
