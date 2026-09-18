#include "tst_result.h"

#include "util/result.h"

#include <QTest>

using State = DataQuality::State;
using Flag = DataQuality::Flag;
using Flags = DataQuality::Flags;

/*!
 * \brief A Good result constructed with flags must not look unflagged: it is promoted to
 * Degraded, and the flags are kept.
 */
void TestResult::constructorPromotesGoodWithFlagsToDegraded()
{
    const ResultDouble result(1.0, State::Good, Flag::Substituted);

    QCOMPARE(result.state(), State::Degraded);
    QCOMPARE(result.flags(), Flags(Flag::Substituted));
    QCOMPARE(result.value(), 1.0);
}

/*!
 * \brief Invalid and NoValue keep the flags they are constructed with, and their state.
 */
void TestResult::constructorKeepsFlagsOnInvalidAndNoValue()
{
    const ResultDouble invalid(0.0, State::Invalid, Flag::OldData);
    QCOMPARE(invalid.state(), State::Invalid);
    QCOMPARE(invalid.flags(), Flags(Flag::OldData));

    const ResultDouble noValue(0.0, State::NoValue, Flag::Blocked);
    QCOMPARE(noValue.state(), State::NoValue);
    QCOMPARE(noValue.flags(), Flags(Flag::Blocked));
}

void TestResult::constructorLeavesUnflaggedGoodAsGood()
{
    const ResultDouble result(1.0, State::Good);

    QCOMPARE(result.state(), State::Good);
    QCOMPARE(result.flags(), Flags(Flag::NoFlags));
}

/*!
 * \brief Setting a new value makes the result a plain Good again, so flags left over from an
 * earlier reading must not survive.
 */
void TestResult::setValueClearsStaleFlags()
{
    ResultDouble result(0.0, State::Invalid, Flag::OldData);

    result.setValue(5.0);

    QCOMPARE(result.state(), State::Good);
    QCOMPARE(result.flags(), Flags(Flag::NoFlags));
    QCOMPARE(result.value(), 5.0);
}

void TestResult::addFlagsAfterSetValueDegrades()
{
    ResultDouble result;

    result.setValue(5.0);
    result.addFlags(Flag::Overflow);

    QCOMPARE(result.state(), State::Degraded);
    QCOMPARE(result.flags(), Flags(Flag::Overflow));
}

QTEST_GUILESS_MAIN(TestResult)
