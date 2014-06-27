#ifndef LABELEDLINEEDIT_H
#define LABELEDLINEEDIT_H

#include <string>
#include <QLineEdit>
#include <QWidget>

namespace Ui {
class LabeledLineEdit;
}

class LabeledLineEdit : public QWidget
{
    Q_OBJECT

private:
    std::string m_text;
    Ui::LabeledLineEdit *m_ui;

public:
    explicit LabeledLineEdit(std::string objectName, std::string label, std::string text, bool key = false);
    ~LabeledLineEdit();

    int getLabelWidth();
    std::string getLabelText();
    std::string getObjectName();
    std::string getText();    
    void setAlignment(int alignment);
    void setLabelText(std::string text);
    void setText(std::string text);

public slots:
    void itemChanged();
    void textChanged();

signals:
    void itemChanged(LabeledLineEdit *item);
};

#endif // LABELEDLINEEDIT_H
