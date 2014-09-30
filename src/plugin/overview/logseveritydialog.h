#ifndef LOGSEVERITYDIALOG_H
#define LOGSEVERITYDIALOG_H

#include <QDialog>

const char *const log_severity[] = {
    "Unknown", "Other",
    "Information",
    "Degraded/Warning",
    "Minor", "Major",
    "Critical",
    "Fatal/NonRecoverable"
};

const char *const syslog_severity[] = {
    "Emergency", "Alert",
    "Critical", "Error",
    "Warning", "Notice",
    "Informational",
    "Debug"
};

namespace Ui
{
class LogSeverityDialog;
}

class LogSeverityDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::LogSeverityDialog *m_ui;

public:
    explicit LogSeverityDialog(QWidget *parent = 0);
    ~LogSeverityDialog();

    int getSeverity();
    std::string getMessage();
    void setMessage(std::string message);

};

#endif // LOGSEVERITYDIALOG_H
