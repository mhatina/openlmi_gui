#include "labeledlineedit.h"
#include "ui_labeledlineedit.h"

LabeledLineEdit::LabeledLineEdit(std::string objectName, std::string label, std::string text, bool key) :
    QWidget(0),
    m_text(text),
    m_ui(new Ui::LabeledLineEdit)
{
    m_ui->setupUi(this);
    setLabelText(label);
    setText(text);    
    if (key) {
        m_ui->lineEdit->setReadOnly(true);
        m_ui->lineEdit->setStyleSheet("QLineEdit{background: pink;}");
    }


    setObjectName(objectName.c_str());

    connect(
        m_ui->lineEdit,
        SIGNAL(textChanged(QString)),
        this,
        SLOT(textChanged()));
    connect(
        m_ui->lineEdit,
        SIGNAL(editingFinished()),
        this,
        SLOT(itemChanged()));    
}

LabeledLineEdit::~LabeledLineEdit()
{
    delete m_ui;
}

int LabeledLineEdit::getLabelWidth()
{
    return m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
}

std::string LabeledLineEdit::getLabelText()
{
    return m_ui->label->text().toStdString();
}

std::string LabeledLineEdit::getObjectName()
{
    return objectName().toStdString();
}

std::string LabeledLineEdit::getText()
{
    return m_ui->lineEdit->text().toStdString();
}

void LabeledLineEdit::setAlignment(int alignment)
{
    int tmp = m_ui->label->fontMetrics().boundingRect(m_ui->label->text()).width();
    if (alignment - tmp > 0)
        m_ui->label->setMinimumWidth(alignment);
}

void LabeledLineEdit::setLabelText(std::string text)
{
    m_ui->label->setText(text.c_str());
}

void LabeledLineEdit::setText(std::string text)
{
    m_text = text;
    m_ui->lineEdit->setText(text.c_str());
}

void LabeledLineEdit::itemChanged()
{
    if (m_ui->lineEdit->text().toStdString() != m_text)
        emit itemChanged(this);
}

void LabeledLineEdit::textChanged()
{
    m_ui->lineEdit->setStyleSheet("QLineEdit{background: yellow;}");
}
