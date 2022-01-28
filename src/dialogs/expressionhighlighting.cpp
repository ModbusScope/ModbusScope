#include "expressionhighlighting.h"
#include "expressionregex.h"

#include <QDebug>

ExpressionHighlighting::ExpressionHighlighting(QTextDocument *parent)
        : QSyntaxHighlighter(parent)
{

    configureConstantNumberRules();
    configureOperatorRules();
    configureRegisterRules();
}

void ExpressionHighlighting::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(_highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    handleRegisterRules(text);
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

    _invalidRegFormat.setForeground(Qt::darkRed);
    _invalidRegFormat.setFontWeight(QFont::Bold);
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
            format =  _invalidRegFormat;
        }
        setFormat(regMatch.capturedStart(), regMatch.capturedLength(), format);
    }
}
