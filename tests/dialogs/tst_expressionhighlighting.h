#ifndef TST_EXPRESSIONHIGHLIGHTING_H
#define TST_EXPRESSIONHIGHLIGHTING_H

#include <QObject>

/*!
 * \brief Tests for the part of an expression that is marked as the error.
 */
class TestExpressionHighlighting : public QObject
{
    Q_OBJECT

private slots:
    void errorSpan_data();
    void errorSpan();
    void parserErrorSpan_data();
    void parserErrorSpan();
};

#endif // TST_EXPRESSIONHIGHLIGHTING_H
