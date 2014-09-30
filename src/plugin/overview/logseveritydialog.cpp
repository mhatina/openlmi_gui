#include "logseveritydialog.h"
#include "ui_logseveritydialog.h"

LogSeverityDialog::LogSeverityDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::LogSeverityDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Popup);
}

LogSeverityDialog::~LogSeverityDialog()
{
    delete m_ui;
}

int LogSeverityDialog::getSeverity()
{
    return m_ui->log_severity->currentIndex();
}

std::string LogSeverityDialog::getMessage()
{
    return m_ui->message->text().toStdString();
}

void LogSeverityDialog::setMessage(std::string message)
{
    m_ui->message->setText(message.c_str());
}
