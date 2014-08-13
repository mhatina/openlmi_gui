#ifndef SYSTEMDETAILSDIALOG_H
#define SYSTEMDETAILSDIALOG_H

#include <QDialog>

#define DEFAULT_TEXT "N/A"

namespace Ui {
class SystemDetailsDialog;
}

class SystemDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemDetailsDialog(QWidget *parent = 0);
    ~SystemDetailsDialog();

    void setDomain(std::string domain);
    void setIpv4(std::string ipv4);
    void setIpv6(std::string ipv6);
    void setMac(std::string mac);
    void reset();

protected:
    void leaveEvent(QEvent *event);

private:
    Ui::SystemDetailsDialog *m_ui;
};

#endif // SYSTEMDETAILSDIALOG_H
