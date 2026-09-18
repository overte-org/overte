//
//  ScriptHighlighting.cpp
//  interface/src
//
//  Created by Thijs Wenker on 4/15/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptHighlighting.h"
#include <QTextDocument>

ScriptHighlighting::ScriptHighlighting(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
    _keywordRegex = QRegularExpression("\\b(break|case|catch|continue|debugger|default|delete|do|else|finally|for|function|if|in|instanceof|new|return|switch|this|throw|try|typeof|var|void|while|with)\\b");
    _quotedTextRegex = QRegularExpression("(\"[^\"]*(\"){0,1}|\'[^\']*(\'){0,1})");
    _multiLineCommentBegin = QRegularExpression("/\\*");
    _multiLineCommentEnd = QRegularExpression("\\*/");
    _numberRegex = QRegularExpression("[0-9]+(\\.[0-9]+){0,1}");
    _singleLineComment = QRegularExpression("//[^\n]*");
    _truefalseRegex = QRegularExpression("\\b(true|false)\\b");
    _alphacharRegex = QRegularExpression("[A-Za-z]");
}

void ScriptHighlighting::highlightBlock(const QString& text) {
    this->highlightKeywords(text);
    this->formatNumbers(text);
    this->formatTrueFalse(text);
    this->formatQuotedText(text);
    this->formatComments(text);
}

void ScriptHighlighting::highlightKeywords(const QString& text) {
    auto keywordMatch = _keywordRegex.match(text);
    while (keywordMatch.hasMatch()) {
        qsizetype index = keywordMatch.capturedStart();
        qsizetype length = keywordMatch.capturedLength();
        setFormat(static_cast<int>(index), static_cast<int>(length), Qt::blue);
        keywordMatch = _keywordRegex.match(text, index + length);
    }
}

void ScriptHighlighting::formatComments(const QString& text) {

    setCurrentBlockState(BlockStateClean);

    qsizetype start = (previousBlockState() != BlockStateInMultiComment) ? text.indexOf(_multiLineCommentBegin) : 0;

    while (start > -1) {
        auto multiLineCommentEndMatch = _multiLineCommentEnd.match(text, start);
        qsizetype end = multiLineCommentEndMatch.capturedEnd();
        qsizetype length = (end == qsizetype(-1) ? text.length() : (end + multiLineCommentEndMatch.capturedLength())) - start;
        setFormat(start, length, Qt::lightGray);
        start = text.indexOf(_multiLineCommentBegin, start + length);
        if (end == -1) {
            setCurrentBlockState(BlockStateInMultiComment);
        }
    }

    auto singleLineCommentMatch = _singleLineComment.match(text);
    qsizetype index = singleLineCommentMatch.capturedStart();
    while (index >= 0) {
        qsizetype length = singleLineCommentMatch.capturedLength();
        auto quotedTextRegexMatch = _quotedTextRegex.match(text);
        qsizetype quoted_index = quotedTextRegexMatch.capturedStart();
        bool valid = true;
        while (quoted_index >= 0 && valid) {
            qsizetype quoted_length = quotedTextRegexMatch.capturedLength();
            if (quoted_index <= index && index <= (quoted_index + quoted_length)) {
                valid = false;
            }
            quotedTextRegexMatch = _quotedTextRegex.match(text, quoted_index + quoted_length);
            quoted_index = quotedTextRegexMatch.capturedStart();
        }

        if (valid) {
            setFormat(index, length, Qt::lightGray);
        }
        singleLineCommentMatch = _singleLineComment.match(text, index + length);
        index = singleLineCommentMatch.capturedStart();
    }
}

void ScriptHighlighting::formatQuotedText(const QString& text){
    auto quotedTextRegexMatch = _quotedTextRegex.match(text);
    qsizetype index = quotedTextRegexMatch.capturedStart();;
    while (index >= 0) {
        qsizetype length = quotedTextRegexMatch.capturedLength();
        setFormat(index, length, Qt::red);
        quotedTextRegexMatch = _quotedTextRegex.match(text, index + length);
        index = quotedTextRegexMatch.capturedStart();
    }
}

void ScriptHighlighting::formatNumbers(const QString& text){
    auto numberRegexMatch = _numberRegex.match(text);
    qsizetype index = numberRegexMatch.capturedStart();
    while (index >= 0) {
        qsizetype length = numberRegexMatch.capturedLength();
        if (index == 0 || text.indexOf(_alphacharRegex, index - 1) != (index - 1)) {
            setFormat(index, length, Qt::green);
        }
        numberRegexMatch = _numberRegex.match(text, index + length);
        index = numberRegexMatch.capturedStart();
    }
}

void ScriptHighlighting::formatTrueFalse(const QString& text){
    auto trueFalseRegexMatch = _truefalseRegex.match(text);
    qsizetype index = trueFalseRegexMatch.capturedStart();
    while (index >= 0) {
        qsizetype length = trueFalseRegexMatch.capturedLength();
        QFont* font = new QFont(this->document()->defaultFont());
        font->setBold(true);
        setFormat(index, length, *font);
        trueFalseRegexMatch = _truefalseRegex.match(text, index + length);
        index = trueFalseRegexMatch.capturedStart();
    }
}
