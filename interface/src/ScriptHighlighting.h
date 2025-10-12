//
//  ScriptHighlighting.h
//  interface/src
//
//  Created by Thijs Wenker on 4/15/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ScriptHighlighting_h
#define hifi_ScriptHighlighting_h

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class ScriptHighlighting : public QSyntaxHighlighter {
    Q_OBJECT

public:
    ScriptHighlighting(QTextDocument* parent = NULL);

    enum BlockState {
        BlockStateClean,
        BlockStateInMultiComment
    };

protected:
    void highlightBlock(const QString& text) override;
    void highlightKeywords(const QString& text);
    void formatComments(const QString& text);
    void formatQuotedText(const QString& text);
    void formatNumbers(const QString& text);
    void formatTrueFalse(const QString& text);

private:
    QRegularExpression _alphacharRegex;
    QRegularExpression _keywordRegex;
    QRegularExpression _quotedTextRegex;
    QRegularExpression _multiLineCommentBegin;
    QRegularExpression _multiLineCommentEnd;
    QRegularExpression _numberRegex;
    QRegularExpression _singleLineComment;
    QRegularExpression _truefalseRegex;
};

#endif // hifi_ScriptHighlighting_h
