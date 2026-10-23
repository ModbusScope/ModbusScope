#include "expressionhighlighting.h"

#include "util/expressionreferences.h"
#include "util/expressionregex.h"

#include <QDebug>
#include <QTextBlock>

ExpressionHighlighting::ExpressionHighlighting(QTextDocument* parent) : QSyntaxHighlighter(parent), _errorPosition(-1)
{

    configureConstantNumberRules();
    configureOperatorRules();
    configureRegisterRules();
}

void ExpressionHighlighting::setExpressionErrorPosition(qint32 pos)
{
    _errorPosition = pos;
}

void ExpressionHighlighting::highlightBlock(const QString& text)
{
    for (const HighlightingRule& rule : std::as_const(_highlightingRules))
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
    const QString numberkeywordPatterns[] = { ExpressionRegex::cNumberDec, ExpressionRegex::cNumberBin,
                                              ExpressionRegex::cNumberHex };

    for (const QString& pattern : numberkeywordPatterns)
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

void ExpressionHighlighting::handleRegisterRules(const QString& text)
{
    QRegularExpressionMatchIterator regMatchIterator = _registerExpression.globalMatch(text);
    while (regMatchIterator.hasNext())
    {
        QRegularExpressionMatch regMatch = regMatchIterator.next();

        QString matched = regMatch.captured();
        QRegularExpressionMatch validRegMatch = _validRegisterExpression.match(matched);

        QTextCharFormat format;
        if (validRegMatch.hasMatch())
        {
            format = _validRegFormat;
        }
        else
        {
            format = _errorFormat;
        }
        setFormat(regMatch.capturedStart(), regMatch.capturedLength(), format);
    }
}

/*!
 * \brief Mark the parser error in the current block, when the error is in that block.
 *
 * The error position counts from the start of the document; a block is one line of it. A position
 * past the end of the text is marked in the last block only.
 * \param text The text of the current block.
 */
void ExpressionHighlighting::handleErrorPosition(const QString& text)
{
    if (_errorPosition < 0)
    {
        return;
    }

    const qint32 blockPosition = _errorPosition - currentBlock().position();
    const bool bLastBlock = !currentBlock().next().isValid();
    if (blockPosition < 0 || (blockPosition >= text.size() && !bLastBlock))
    {
        return;
    }

    const ExpressionErrorMark::Span mark = ExpressionErrorMark::span(blockPosition, text);
    if (mark.start >= 0)
    {
        setFormat(static_cast<int>(mark.start), static_cast<int>(mark.length), _errorFormat);
    }
}

namespace ExpressionErrorMark {

/*!
 * \brief Returns the part of \a text to mark for a parser error at \a errorPos.
 *
 * The parser reports the position of the first character of the token it did not expect. The mark
 * covers that token: a data point reference, a number or a name, else one character. A position at
 * or past the end of the text (an unexpected end of the expression) marks the last character that
 * is not a space.
 * \param errorPos The error position reported by the parser, negative when there is no error.
 * \param text     The expression text.
 * \return The span to mark; \c start is -1 when there is nothing to mark.
 */
Span span(qint32 errorPos, const QString& text)
{
    static const QRegularExpression tokenRegex(R"(0[xX][0-9a-fA-F]+|0[bB][01]+|\d+(?:[.,]\d+)?|[A-Za-z_]\w*)");

    if (errorPos < 0)
    {
        return { -1, 0 };
    }

    if (errorPos >= text.size())
    {
        qsizetype last = text.size() - 1;
        while (last >= 0 && text.at(last).isSpace())
        {
            last--;
        }
        return { last, last >= 0 ? 1 : 0 };
    }

    const QList<ExpressionReferences::Reference> references = ExpressionReferences::find(text);
    for (const ExpressionReferences::Reference& reference : references)
    {
        if (errorPos >= reference.start && errorPos < reference.start + reference.length)
        {
            return { reference.start, reference.length };
        }
    }

    const QRegularExpressionMatch token =
      tokenRegex.match(text, errorPos, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
    if (token.hasMatch())
    {
        return { errorPos, token.capturedLength() };
    }

    return { errorPos, 1 };
}

} // namespace ExpressionErrorMark
