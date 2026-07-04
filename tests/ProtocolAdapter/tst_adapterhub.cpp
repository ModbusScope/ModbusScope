#include "tst_adapterhub.h"

#include "ProtocolAdapter/adapterhub.h"

#include <QSignalSpy>
#include <QTest>

/*!
 * \brief A single pending adapter that fails to start must only report sessionError.
 *
 * Matches today's single-adapter (modbus-only) behavior and must stay correct.
 */
void TestAdapterHub::errorOnSingleAdapterEmitsErrorOnly()
{
    AdapterHub hub;
    hub._pendingStartAdapters.insert(QStringLiteral("modbus"));

    QSignalSpy startedSpy(&hub, &AdapterHub::sessionStarted);
    QSignalSpy errorSpy(&hub, &AdapterHub::sessionError);

    hub.onManagerSessionError(QStringLiteral("modbus"), QStringLiteral("boom"));

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(startedSpy.count(), 0);
}

/*!
 * \brief Regression test: a sibling adapter having already started must not turn a
 * later sibling's failure into a reported success.
 */
void TestAdapterHub::errorOnLastPendingAdapterDoesNotEmitSessionStarted()
{
    AdapterHub hub;
    hub._pendingStartAdapters.insert(QStringLiteral("modbus"));
    hub._pendingStartAdapters.insert(QStringLiteral("sim"));

    QSignalSpy startedSpy(&hub, &AdapterHub::sessionStarted);
    QSignalSpy errorSpy(&hub, &AdapterHub::sessionError);

    /* First adapter starts successfully - still waiting on "sim" */
    hub.onManagerSessionStarted(QStringLiteral("modbus"));
    QCOMPARE(startedSpy.count(), 0);

    /* Second (last pending) adapter fails to start */
    hub.onManagerSessionError(QStringLiteral("sim"), QStringLiteral("boom"));

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(startedSpy.count(), 0);
}

/*!
 * \brief Happy-path guard: sessionStarted is still emitted exactly once when all
 * pending adapters start successfully.
 */
void TestAdapterHub::allAdaptersSucceedEmitsSessionStartedOnce()
{
    AdapterHub hub;
    hub._pendingStartAdapters.insert(QStringLiteral("modbus"));
    hub._pendingStartAdapters.insert(QStringLiteral("sim"));

    QSignalSpy startedSpy(&hub, &AdapterHub::sessionStarted);

    hub.onManagerSessionStarted(QStringLiteral("modbus"));
    QCOMPARE(startedSpy.count(), 0);

    hub.onManagerSessionStarted(QStringLiteral("sim"));
    QCOMPARE(startedSpy.count(), 1);
}

QTEST_GUILESS_MAIN(TestAdapterHub)
