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

/*!
 * \brief Requesting Good on a result that carries flags must keep it Degraded, so flagged data
 * never looks clean.
 */
void TestResult::setStateGoodWithFlagsStaysDegraded()
{
    ResultDouble result(1.0, State::Degraded, Flag::Substituted);

    result.setState(State::Good);

    QCOMPARE(result.state(), State::Degraded);
    QCOMPARE(result.flags(), Flags(Flag::Substituted));
}

void TestResult::setStateGoodWithoutFlagsIsGood()
{
    ResultDouble result(1.0, State::Invalid);

    result.setState(State::Good);

    QCOMPARE(result.state(), State::Good);
    QCOMPARE(result.flags(), Flags(Flag::NoFlags));
}

void TestResult::setStateKeepsFlagsOnOtherStates()
{
    ResultDouble result(1.0, State::Degraded, Flag::OldData);

    result.setState(State::Invalid);

    QCOMPARE(result.state(), State::Invalid);
    QCOMPARE(result.flags(), Flags(Flag::OldData));
}

/*!
 * \brief The numeric codes are part of the data file format and must stay stable.
 */
void TestResult::exportCodeValues()
{
    using DataQuality::Quality;
    using DataQuality::toExportCode;

    QCOMPARE(toExportCode(Quality{ State::Good, Flag::NoFlags }), 0u);
    QCOMPARE(toExportCode(Quality{ State::Degraded, Flag::NoFlags }), 1u);
    QCOMPARE(toExportCode(Quality{ State::Invalid, Flag::NoFlags }), 2u);
    QCOMPARE(toExportCode(Quality{ State::NoValue, Flag::NoFlags }), 3u);

    QCOMPARE(toExportCode(Quality{ State::Degraded, Flag::Substituted }), 17u);
    QCOMPARE(toExportCode(Quality{ State::Degraded, Flag::Blocked }), 33u);
    QCOMPARE(toExportCode(Quality{ State::Degraded, Flag::Overflow }), 65u);
    QCOMPARE(toExportCode(Quality{ State::Degraded, Flag::OldData }), 129u);
}

void TestResult::exportCodeRoundTrip()
{
    const QList<State> states{ State::Good, State::Degraded, State::Invalid, State::NoValue };

    for (State state : states)
    {
        for (quint32 flagBits = 0; flagBits < 16; flagBits++)
        {
            const Flags flags = Flags::fromInt(flagBits);
            if (state == State::Good && flags)
            {
                continue;
            }

            const DataQuality::Quality quality{ state, flags };
            QCOMPARE(DataQuality::fromExportCode(DataQuality::toExportCode(quality)), quality);
        }
    }
}

void TestResult::exportCodeIgnoresUnknownFlagBits()
{
    const DataQuality::Quality quality = DataQuality::fromExportCode(1u | (1u << 4) | (1u << 20));

    QCOMPARE(quality.state, State::Degraded);
    QCOMPARE(quality.flags, Flags(Flag::Substituted));
}

void TestResult::exportCodeUnknownStateIsInvalid()
{
    const DataQuality::Quality quality = DataQuality::fromExportCode(7u);

    QCOMPARE(quality.state, State::Invalid);
    QCOMPARE(quality.flags, Flags(Flag::NoFlags));
}

void TestResult::exportCodeGoodWithFlagsIsDegraded()
{
    const DataQuality::Quality quality = DataQuality::fromExportCode(128u);

    QCOMPARE(quality.state, State::Degraded);
    QCOMPARE(quality.flags, Flags(Flag::OldData));
}

QTEST_GUILESS_MAIN(TestResult)
