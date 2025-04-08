#include "expressionhighlighting.h"

#include "util/expressionregex.h"

#include <QDebug>

ExpressionHighlighting::ExpressionHighlighting(QTextDocument *parent)
        : QSyntaxHighlighter(parent), _errorPosition(-1)
{

    configureConstantNumberRules();
    configureOperatorRules();
    configureRegisterRules();
}

void ExpressionHighlighting::setExpressionErrorPosition(qint32 pos)
{
    _errorPosition = pos;
}

void ExpressionHighlighting::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const(_highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    handleRegisterRules(text);
    handleErrorPosition(text);
}

void ExpressionHighlighting::configureConstantNumberRules()
{
    HighlightingRule rule;

    QTextCharFormat numberwordFormat;
    numberwordFormat.setForeground(Qt::darkBlue);
    numberwordFormat.setFontWeight(QFont::Bold);
    const QString numberkeywordPatterns[] = {
        ExpressionRegex::cNumberDec,
        ExpressionRegex::cNumberBin,
        ExpressionRegex::cNumberHex
    };

    for (const QString &pattern : numberkeywordPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = numberwordFormat;
        _highlightingRules.append(rule);
    }
}

void ExpressionHighlighting::configureOperatorRules()
{
    HighlightingRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::gray);
    keywordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(ExpressionRegex::cOperatorcharacters);
    rule.format = keywordFormat;
    _highlightingRules.append(rule);
}

void ExpressionHighlighting::configureRegisterRules()
{
    _validRegFormat.setForeground(Qt::darkGreen);
    _validRegFormat.setFontWeight(QFont::Bold);
    _registerExpression = QRegularExpression(ExpressionRegex::cMatchRegister);

    _errorFormat.setForeground(Qt::darkRed);
    _errorFormat.setFontWeight(QFont::Bold);
    _validRegisterExpression = QRegularExpression(ExpressionRegex::cParseReg);
}

void ExpressionHighlighting::handleRegisterRules(const QString &text)
{
    QRegularExpressionMatchIterator regMatchIterator = _registerExpression.globalMatch(text);
    while (regMatchIterator.hasNext())
    {
        QRegularExpressionMatch regMatch = regMatchIterator.next();

        QString matched= regMatch.captured();
        QRegularExpressionMatch validRegMatch = _validRegisterExpression.match(matched);

        QTextCharFormat format;
        if (validRegMatch.hasMatch())
        {
            format = _validRegFormat;
        }
        else
        {
            format =  _errorFormat;
        }
        setFormat(regMatch.capturedStart(), regMatch.capturedLength(), format);
    }
}

void ExpressionHighlighting::handleErrorPosition(const QString &text)
{
    int correctedErrorPos;
    if (_errorPosition < 0)
    {
        correctedErrorPos = _errorPosition;
    }
    else if (_errorPosition < 2)
    {
        correctedErrorPos = 0;
    }
    else
    {
        correctedErrorPos = _errorPosition - 2; /* Counting from zero + error pos is position of unexpected character */
    }

    if (
        (correctedErrorPos >= 0)
        && (correctedErrorPos < text.size())
    )
    {
        setFormat(correctedErrorPos, text.size() - correctedErrorPos, _errorFormat);
    }
}
