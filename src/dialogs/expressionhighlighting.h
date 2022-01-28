#ifndef EXPRESSIONHIGHLIGHTING_H
#define EXPRESSIONHIGHLIGHTING_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class ExpressionHighlighting : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit ExpressionHighlighting(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void configureConstantNumberRules();
    void configureOperatorRules();
    void configureRegisterRules();

    void handleRegisterRules(const QString &text);

    QVector<HighlightingRule> _highlightingRules;

    QRegularExpression _registerExpression;
    QTextCharFormat _validRegFormat;

    QRegularExpression _validRegisterExpression;
    QTextCharFormat _invalidRegFormat;

};

#endif // EXPRESSIONHIGHLIGHTING_H
