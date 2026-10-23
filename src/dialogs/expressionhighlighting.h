#ifndef EXPRESSIONHIGHLIGHTING_H
#define EXPRESSIONHIGHLIGHTING_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace ExpressionErrorMark {

//! Part of the expression text to mark as the error.
struct Span
{
    qsizetype start; //!< -1 when there is nothing to mark
    qsizetype length;
};

Span span(qint32 errorPos, const QString& text);

} // namespace ExpressionErrorMark

class ExpressionHighlighting : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit ExpressionHighlighting(QTextDocument* parent = nullptr);

    void setExpressionErrorPosition(qint32 pos);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void configureConstantNumberRules();
    void configureOperatorRules();
    void configureRegisterRules();

    void handleRegisterRules(const QString& text);
    void handleErrorPosition(const QString& text);

    qint32 _errorPosition;

    QVector<HighlightingRule> _highlightingRules;

    QRegularExpression _registerExpression;
    QTextCharFormat _validRegFormat;

    QRegularExpression _validRegisterExpression;
    QTextCharFormat _errorFormat;
};

#endif // EXPRESSIONHIGHLIGHTING_H
