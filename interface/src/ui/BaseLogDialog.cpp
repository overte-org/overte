//
//  BaseLogDialog.cpp
//  interface/src/ui
//
//  Created by Clement Brisset on 1/31/17.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "BaseLogDialog.h"

#include <QDir>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QPushButton>
#include <QRegularExpression>

#include <PathUtils.h>

const int TOP_BAR_HEIGHT = 124;
const int INITIAL_WIDTH = 800;
const int INITIAL_HEIGHT = 480;
const int MINIMAL_WIDTH = 780;
const int SEARCH_BUTTON_WIDTH = 32;
const int SEARCH_TOGGLE_BUTTON_WIDTH = 50;
const int SEARCH_TEXT_WIDTH = 240;
const int TIME_STAMP_LENGTH = 16;
const int FONT_WEIGHT = 75;
const QColor HIGHLIGHT_COLOR = QColor("#00B4EF");
const QColor BOLD_COLOR = QColor("#1080B8");
const QString BOLD_PATTERN = "\\[\\d*\\/.*:\\d*:\\d*\\]";

BaseLogDialog::BaseLogDialog(QWidget* parent) : QDialog(parent, Qt::Window) {
    setWindowTitle("Base Log");
    setAttribute(Qt::WA_DeleteOnClose);

    initControls();

    resize(INITIAL_WIDTH, INITIAL_HEIGHT);
    setMinimumWidth(MINIMAL_WIDTH);
}

BaseLogDialog::~BaseLogDialog() {
    deleteLater();
}

void BaseLogDialog::initControls() {
    _searchButton = new QPushButton(QIcon(":/styles/search.svg"), "", this);
    // set object name for css styling
    _searchButton->setObjectName("searchButton");
    _leftPad = 8;
    _searchButton->setGeometry(_leftPad, ELEMENT_MARGIN, SEARCH_BUTTON_WIDTH, ELEMENT_HEIGHT);
    _leftPad += SEARCH_BUTTON_WIDTH;
    _searchButton->show();
    connect(_searchButton, &QPushButton::clicked, this, &BaseLogDialog::handleSearchButton);

    _searchTextBox = new QLineEdit(this);
    _searchTextBox->setGeometry(_leftPad, ELEMENT_MARGIN, SEARCH_TEXT_WIDTH, ELEMENT_HEIGHT);
    _leftPad += SEARCH_TEXT_WIDTH + BUTTON_MARGIN;
    _searchTextBox->show();
    connect(_searchTextBox, &QLineEdit::textChanged, this, &BaseLogDialog::handleSearchTextChanged);
    connect(_searchTextBox, &QLineEdit::returnPressed, this, &BaseLogDialog::toggleSearchNext);

    _searchPrevButton = new QPushButton(this);
    _searchPrevButton->setObjectName("searchPrevButton");
    _searchPrevButton->setGeometry(_leftPad, ELEMENT_MARGIN, SEARCH_TOGGLE_BUTTON_WIDTH, ELEMENT_HEIGHT);
    _searchPrevButton->setText("Prev");
    _leftPad += SEARCH_TOGGLE_BUTTON_WIDTH + BUTTON_MARGIN;
    _searchPrevButton->show();
    connect(_searchPrevButton, &QPushButton::clicked, this, &BaseLogDialog::toggleSearchPrev);

    _searchNextButton = new QPushButton(this);
    _searchNextButton->setObjectName("searchNextButton");
    _searchNextButton->setGeometry(_leftPad, ELEMENT_MARGIN, SEARCH_TOGGLE_BUTTON_WIDTH, ELEMENT_HEIGHT);
    _searchNextButton->setText("Next");
    _leftPad += SEARCH_TOGGLE_BUTTON_WIDTH + CHECKBOX_MARGIN;
    _searchNextButton->show();
    connect(_searchNextButton, &QPushButton::clicked, this, &BaseLogDialog::toggleSearchNext);

    _logTextBox = new QPlainTextEdit(this);
    _logTextBox->setFont(QFont("monospace"));
    _logTextBox->setReadOnly(true);
    _logTextBox->show();
    _highlighter = new Highlighter(_logTextBox->document());
    connect(_logTextBox, &QPlainTextEdit::selectionChanged, this, &BaseLogDialog::updateSelection);
}

void BaseLogDialog::showEvent(QShowEvent* event) {
    showLogData();
}

void BaseLogDialog::resizeEvent(QResizeEvent* event) {
    _logTextBox->setGeometry(0, TOP_BAR_HEIGHT, width(), height() - TOP_BAR_HEIGHT);
}

void BaseLogDialog::appendLogLine(QString logLine) {
    if (logLine.contains(_searchTerm, Qt::CaseInsensitive)) {
        int indexToBold = _logTextBox->document()->characterCount();
        _logTextBox->appendPlainText(logLine.trimmed());
        _highlighter->setBold(indexToBold);
    }
}

void BaseLogDialog::handleSearchButton() {
    _searchTextBox->setFocus();
}

void BaseLogDialog::handleSearchTextChanged(QString searchText) {
    QTextCursor cursor = _logTextBox->textCursor();

    if (cursor.hasSelection()) {
        QString selectedTerm = cursor.selectedText();
        if (selectedTerm == searchText) {
            return;
        }
    }

    cursor.setPosition(0, QTextCursor::MoveAnchor);
    _logTextBox->setTextCursor(cursor);
    bool foundTerm = _logTextBox->find(searchText);

    if (!foundTerm) {
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        _logTextBox->setTextCursor(cursor);
    }

    _searchTerm = searchText;
    _highlighter->keyword = searchText;
    _highlighter->rehighlight();
}

void BaseLogDialog::clearSearch() {
    _searchTextBox->setText("");
}

void BaseLogDialog::toggleSearchPrev() {
    QTextCursor searchCursor = _logTextBox->textCursor();
    if (searchCursor.hasSelection()) {
        QString selectedTerm = searchCursor.selectedText();
        _logTextBox->find(selectedTerm, QTextDocument::FindBackward);
    } else {
        handleSearchTextChanged(_searchTextBox->text());
    }
}

void BaseLogDialog::toggleSearchNext() {
    QTextCursor searchCursor = _logTextBox->textCursor();
    if (searchCursor.hasSelection()) {
        QString selectedTerm = searchCursor.selectedText();
        _logTextBox->find(selectedTerm);
    } else {
        handleSearchTextChanged(_searchTextBox->text());
    }
}

void BaseLogDialog::showLogData() {
    _logTextBox->clear();
    _logTextBox->appendPlainText(getCurrentLog());
    _logTextBox->ensureCursorVisible();
    _highlighter->rehighlight();
}

void BaseLogDialog::updateSelection() {
    QTextCursor cursor = _logTextBox->textCursor();
    if (cursor.hasSelection()) {
        QString selectionTerm = cursor.selectedText();
        if (QString::compare(selectionTerm, _searchTextBox->text(), Qt::CaseInsensitive) != 0) {
            _searchTextBox->setText(selectionTerm);
        }
    }
}

Highlighter::Highlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    boldFormat.setFontWeight(FONT_WEIGHT);
    boldFormat.setForeground(BOLD_COLOR);
    keywordFormat.setFontWeight(FONT_WEIGHT);
    keywordFormat.setForeground(HIGHLIGHT_COLOR);
}

void Highlighter::highlightBlock(const QString& text) {
    QRegularExpression expression(BOLD_PATTERN);

    auto expressionMatch = expression.match(text);
    qsizetype index = expressionMatch.capturedStart();

    while (index >= 0) {
        qsizetype length = expressionMatch.capturedLength();
        setFormat(index, length, boldFormat);
        expressionMatch = expression.match(text, index + length);
        index = expressionMatch.capturedStart();
    }

    if (keyword.isNull() || keyword.isEmpty()) {
        return;
    }

    index = text.indexOf(keyword, 0, Qt::CaseInsensitive);
    int length = keyword.length();

    while (index >= 0) {
        setFormat(index, length, keywordFormat);
        index = text.indexOf(keyword, index + length, Qt::CaseInsensitive);
    }
}

void Highlighter::setBold(int indexToBold) {
    setFormat(indexToBold, TIME_STAMP_LENGTH, boldFormat);
}
