#ifndef TST_EXPRESSIONVALUE_H
#define TST_EXPRESSIONVALUE_H

#include <QObject>

/*!
 * \brief Tests for parsing the example values of the Compose expression dialog.
 */
class TestExpressionValue : public QObject
{
    Q_OBJECT

private slots:
    void parseValid_data();
    void parseValid();
    void parseInvalid_data();
    void parseInvalid();
    void parseNegativeZero();
    void parseWithoutOk();
};

#endif // TST_EXPRESSIONVALUE_H
