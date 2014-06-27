#include "detailsdialog.h"
#include "instructions/changegrouppropertyinstruction.h"
#include "instructions/changeuserpropertyinstruction.h"
#include "labeledlineedit.h"
#include "ui_detailsdialog.h"

#include <lmiwbem_value.h>
#include <QLabel>
#include <QTextEdit>

bool DetailsDialog::isKeyProperty(const char *property)
{
    int cnt = sizeof(key_property) / sizeof(key_property[0]);
    for (int i = 0; i < cnt; i++) {
        if (strcmp(key_property[i], property) == 0)
            return true;
    }

    return false;
}

std::string DetailsDialog::insertSpaces(std::string text)
{
    for (unsigned int i = 1; i < text.length(); i++) {
        if (isupper(text[i]) && islower(text[i - 1])) {
            text.insert(i, " ");
            i++;
        }
    }

    return text;
}

DetailsDialog::DetailsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DetailsDialog)
{
    m_ui->setupUi(this);    
}

DetailsDialog::~DetailsDialog()
{
    delete m_ui;
}

std::map<std::string, std::string> DetailsDialog::getChanges()
{
    return m_changes;
}

void DetailsDialog::alterProperties(std::vector<IInstruction *> instructions)
{
    int cnt = instructions.size();

    Pegasus::Uint32 propIndex = m_instance.findProperty(Pegasus::CIMName("Name"));
    Pegasus::CIMProperty prop = m_instance.getProperty(propIndex);
    Pegasus::CIMValue value = prop.getValue();
    std::string name = CIMValue::to_std_string(value);
    std::string property_name;
    std::string str_value;

    for (int i = 0; i < cnt; i++) {
        bool found = false;
        IInstruction *instruction = instructions[i];
        property_name = instruction->getInstructionName();
        str_value = CIMValue::to_std_string(instruction->getValue());

        if (instruction->getSubject() == IInstruction::ACCOUNT
                && m_instance.findProperty(Pegasus::CIMName(property_name.c_str())) != Pegasus::Uint32(-1))
            found = ((ChangeUserPropertyInstruction*) instructions[i])->getUserName() == name;
        else if (instruction->getSubject() == IInstruction::GROUP
                 && m_instance.findProperty(Pegasus::CIMName(property_name.c_str())) != Pegasus::Uint32(-1))
            found = ((ChangeGroupPropertyInstruction*) instructions[i])->getGroupName() == name;

        if (!found)
            continue;

        LabeledLineEdit *line;
        if ((line = findChild<LabeledLineEdit*>(property_name.c_str())) != NULL) {
            line->textChanged();
            line->setText(str_value);
        }
     }
}

void DetailsDialog::setInstance(Pegasus::CIMInstance instance)
{
    m_changes_enabled = false;
    m_instance = instance;

    std::vector<LabeledLineEdit*> lines;
    int cnt = instance.getPropertyCount();
    int max_width = 0;
    for (int i = 0; i < cnt; i++) {
        std::string object_name = std::string(instance.getProperty(i).getName().getString().getCString());
        std::string str_name = insertSpaces(object_name);
        std::string str_value = CIMValue::to_std_string(instance.getProperty(i).getValue());
        QWidget *widget_area = findChild<QWidget*>("widget_area");
        QLayout *layout = widget_area->layout();

        LabeledLineEdit *widget = new LabeledLineEdit(object_name, str_name, str_value, isKeyProperty(object_name.c_str()));
        if (max_width < widget->getLabelWidth())
            max_width = widget->getLabelWidth();
        connect(
            widget,
            SIGNAL(itemChanged(LabeledLineEdit*)),
            this,
            SLOT(itemChanged(LabeledLineEdit*)));
        layout->addWidget(widget);
        lines.push_back(widget);
    }
    for (int i = 0; i < cnt; i++)
        lines[i]->setAlignment(max_width);

    m_changes_enabled = true;
}

void DetailsDialog::itemChanged(LabeledLineEdit *item)
{
    if (!m_changes_enabled)
        return;

    std::string property_name = item->getObjectName();
    std::string value = item->getText();
    m_changes[property_name] = value;
}
