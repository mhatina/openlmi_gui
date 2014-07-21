#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QList>
#include <QListWidget>
#include <QWidget>

namespace Ui {
class ListWidget;
}

class ListWidget : public QListWidget
{
    Q_OBJECT

private:
    Ui::ListWidget *m_ui;

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

public:
    explicit ListWidget(QWidget *parent = 0);
    ~ListWidget();

    QList<QListWidgetItem*> selectedItems() const;
    void addItem(QListWidgetItem *item);
    void removeItem(QListWidgetItem *item);

signals:
    void getFocus();
    void lostFocus();
};

#endif // LISTWIDGET_H
