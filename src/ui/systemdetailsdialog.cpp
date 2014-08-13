#include "systemdetailsdialog.h"
#include "ui_systemdetailsdialog.h"

SystemDetailsDialog::SystemDetailsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SystemDetailsDialog)
{
    m_ui->setupUi(this);
}

SystemDetailsDialog::~SystemDetailsDialog()
{
    delete m_ui;
}

void SystemDetailsDialog::setDomain(std::string domain)
{
    m_ui->domain->setText(domain.c_str());
}

void SystemDetailsDialog::setIpv4(std::string ipv4)
{
    m_ui->ipv4->setText(ipv4.c_str());
}

void SystemDetailsDialog::setIpv6(std::string ipv6)
{
    m_ui->ipv6->setText(ipv6.c_str());
}

void SystemDetailsDialog::setMac(std::string mac)
{
    m_ui->mac->setText(mac.c_str());
}

void SystemDetailsDialog::reset()
{
    m_ui->domain->setText(DEFAULT_TEXT);
    m_ui->ipv4->setText(DEFAULT_TEXT);
    m_ui->ipv6->setText(DEFAULT_TEXT);
    m_ui->mac->setText(DEFAULT_TEXT);
}

void SystemDetailsDialog::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    hide();
}
