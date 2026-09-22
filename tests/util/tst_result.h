#ifndef TST_RESULT_H
#define TST_RESULT_H

#include <QObject>

/*!
 * \brief Tests for the Result<T> data-quality contract: a Good result never carries flags.
 */
class TestResult : public QObject
{
    Q_OBJECT

private slots:
    void constructorPromotesGoodWithFlagsToDegraded();
    void constructorKeepsFlagsOnInvalidAndNoValue();
    void constructorLeavesUnflaggedGoodAsGood();
    void setValueClearsStaleFlags();
    void addFlagsAfterSetValueDegrades();
    void setStateGoodWithFlagsStaysDegraded();
    void setStateGoodWithoutFlagsIsGood();
    void setStateKeepsFlagsOnOtherStates();
};

#endif // TST_RESULT_H
