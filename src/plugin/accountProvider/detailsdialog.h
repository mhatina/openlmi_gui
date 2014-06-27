#ifndef DETAILSDIALOG_H
#define DETAILSDIALOG_H

#include "instructions/instruction.h"
#include "labeledlineedit.h"

#include <Pegasus/Common/CIMInstance.h>
#include <QDialog>
#include <QTableWidgetItem>

const char* const key_property[] = {
    "CreationClassName",
    "Name",
    "SystemCreationClassName",
    "SystemName"
};

namespace Ui {
class DetailsDialog;
}

class DetailsDialog : public QDialog
{
    Q_OBJECT

private:    
    bool m_changes_enabled;
    Pegasus::CIMInstance m_instance;
    std::map<std::string, std::string> m_changes;
    Ui::DetailsDialog *m_ui;

    bool isKeyProperty(const char* property);
    std::string insertSpaces(std::string text);

public:
    explicit DetailsDialog(QWidget *parent = 0);
    ~DetailsDialog();

    std::map<std::string, std::string> getChanges();
    void alterProperties(std::vector<IInstruction*> instructions);
    void setInstance(Pegasus::CIMInstance instance);

private slots:
    void itemChanged(LabeledLineEdit *item);
};

#endif // DETAILSDIALOG_H
