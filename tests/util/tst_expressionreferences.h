#ifndef TST_EXPRESSIONREFERENCES_H
#define TST_EXPRESSIONREFERENCES_H

#include <QObject>

/*!
 * \brief Tests for finding and replacing data point references in expression text.
 */
class TestExpressionReferences : public QObject
{
    Q_OBJECT

private slots:
    void findNone();
    void findOne();
    void findDuplicates();
    void findWithWhitespaceAndDevice();
    void findInvalidContent();
    void replaceSecondOfTwoIdentical();
    void replaceKeepsSurroundingText();
    void replaceStaleReference();
};

#endif // TST_EXPRESSIONREFERENCES_H
