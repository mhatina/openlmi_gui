/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "logger.h"
#include "codedialog.h"
#include "ui_codedialog.h"

#include <QFile>
#include <Qsci/qsciapis.h>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

void CodeDialog::initEditor()
{
    // caret line
    m_ui->code_editor->setCaretLineVisible(true);
    m_ui->code_editor->setCaretLineBackgroundColor(QColor("#d0d0d0"));

    // folding
    QsciScintilla::FoldStyle state =
        static_cast<QsciScintilla::FoldStyle>((!m_ui->code_editor->folding()) * 5);
    if (!state) {
        m_ui->code_editor->foldAll(false);
    }
    m_ui->code_editor->setFolding(state);

    // font
    QFont font("Monospace", 8);
    font.setFixedPitch(true);
    m_ui->code_editor->setFont(font);

    // lexer
    QsciLexerCPP *lexer = new QsciLexerCPP();
    lexer->setDefaultFont(m_ui->code_editor->font());
    lexer->setFoldComments(true);
    m_ui->code_editor->setLexer(lexer);

    // margin
    QFontMetrics fontmetrics = QFontMetrics(m_ui->code_editor->font());
    m_ui->code_editor->setMarginsFont(m_ui->code_editor->font());
    m_ui->code_editor->setMarginWidth(0, fontmetrics.width(QString::number(m_ui->code_editor->lines())) + 6);
    m_ui->code_editor->setMarginLineNumbers(0, true);
    m_ui->code_editor->setMarginsBackgroundColor(QColor("#cccccc"));

    connect(
        m_ui->code_editor,
        SIGNAL(textChanged()),
        this,
        SLOT(textChanged()));

    // auto completion
    QsciAPIs *api = new QsciAPIs(lexer);

    // TODO load strings from library
    while (0) {
        api->add("");
    }
    api->prepare();

    m_ui->code_editor->setAutoCompletionThreshold(1);
    m_ui->code_editor->setAutoCompletionSource(QsciScintilla::AcsAPIs);
}

void CodeDialog::textChanged()
{
    QFontMetrics fontmetrics = m_ui->code_editor->fontMetrics();
    m_ui->code_editor->setMarginWidth(0, fontmetrics.width(QString::number(m_ui->code_editor->lines())) + 6);
}

CodeDialog::CodeDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CodeDialog)
{
    Logger::getInstance()->debug("CodeDialog::CodeDialog(QWidget *parent)");
    m_ui->setupUi(this);

    initEditor();
}

CodeDialog::~CodeDialog()
{
    Logger::getInstance()->debug("CodeDialog::~CodeDialog()");
    delete m_ui;
}

void CodeDialog::setText(String text)
{
    Logger::getInstance()->debug("CodeDialog::setText(String text)");
    text = "#!/usr/bin/lmishell\n\n" + text;
    m_ui->code_editor->setText(text);
    int last_line = m_ui->code_editor->lines();
    m_ui->code_editor->setCursorPosition(last_line, 0);
}
