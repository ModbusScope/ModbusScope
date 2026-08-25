#include "tst_adapterhub.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"

#include <QSignalSpy>
#include <QTest>

namespace {

/*! \brief Minimal AdapterManager double for testing AdapterHub's per-manager iteration logic. */
class FakeAdapterManager : public AdapterManager
{
public:
    /*! \brief Mirrors the AdapterClient states AdapterHub distinguishes between. */
    enum class FakeState
    {
        Idle,
        AwaitingConfig, /*!< Discovered but never started a session. */
        MidHandshake,   /*!< CONFIGURING/STARTING: session requested, not yet established. */
        Active
    };

    FakeAdapterManager(const QString& id, FakeState state, QObject* parent)
        : AdapterManager(id, QString(), nullptr, parent), _state(state)
    {
    }

    bool isAdapterIdle() const override
    {
        return _state == FakeState::Idle;
    }
    bool isAdapterReady() const override
    {
        return _state == FakeState::AwaitingConfig;
    }
    bool isAdapterActive() const override
    {
        return _state == FakeState::Active;
    }
    void requestReadData() override
    {
        _readDataCalls++;
    }
    void stopSession() override
    {
        _stopSessionCalls++;
        if (_emitsAdapterReadySynchronouslyOnStop)
        {
            /* Mirrors AdapterClient::stopSession() for ACTIVE_DEGRADED, which transitions
               locally and emits adapterReady() synchronously instead of waiting on a real
               adapter.stop round-trip. */
            emit adapterReady();
        }
    }
    void initAdapter() override
    {
        _initAdapterCalls++;
    }

    int readDataCalls() const
    {
        return _readDataCalls;
    }
    int stopSessionCalls() const
    {
        return _stopSessionCalls;
    }
    int initAdapterCalls() const
    {
        return _initAdapterCalls;
    }

    void setEmitsAdapterReadySynchronouslyOnStop(bool emitsSynchronously)
    {
        _emitsAdapterReadySynchronouslyOnStop = emitsSynchronously;
    }

private:
    FakeState _state;
    int _readDataCalls = 0;
    int _stopSessionCalls = 0;
    int _initAdapterCalls = 0;
    bool _emitsAdapterReadySynchronouslyOnStop = false;
};

} // namespace

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

/*!
 * \brief requestReadData() must only reach adapters that have an established session -
 * an adapter with no configured registers is discovered and initialized but never
 * started, and must not be polled (regression test for spurious "non-active state"
 * warnings being logged every poll cycle).
 */
void TestAdapterHub::requestReadDataSkipsInactiveAdapters()
{
    AdapterHub hub;
    auto* pActive = new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::Active, &hub);
    auto* pInactive =
      new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::AwaitingConfig, &hub);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pActive);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pInactive);

    hub.requestReadData();

    QCOMPARE(pActive->readDataCalls(), 1);
    QCOMPARE(pInactive->readDataCalls(), 0);
}

/*!
 * \brief stopSession() must only reach and wait on adapters that have an established
 * session - an adapter that was discovered but never started must be left alone (it is
 * already in a state ready for the next session). Regression test: previously every
 * manager's id was added to _pendingReadyAdapters and stopSession() force-killed the
 * never-started adapter's process, but that path never emits adapterReady(), so
 * _pendingReadyAdapters could never empty again and adapterReady() would never re-fire.
 */
void TestAdapterHub::stopSessionOnlyWaitsOnActiveAdapters()
{
    AdapterHub hub;
    auto* pActive = new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::Active, &hub);
    auto* pInactive =
      new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::AwaitingConfig, &hub);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pActive);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pInactive);

    QSignalSpy readySpy(&hub, &AdapterHub::adapterReady);

    hub.stopSession();

    QCOMPARE(pActive->stopSessionCalls(), 1);
    QCOMPARE(pInactive->stopSessionCalls(), 0);

    /* Only the active adapter's graceful stop leads to adapterReady(). If the inactive
       adapter had also been added to _pendingReadyAdapters, this would never fire. */
    hub.onManagerAdapterReady(QStringLiteral("iec104"));
    QCOMPARE(readySpy.count(), 1);
}

/*!
 * \brief stopSession() must force-stop an adapter that is mid-handshake (session requested via
 * startSession() but not yet ACTIVE) without waiting on it, since a force-killed mid-handshake
 * adapter only emits sessionStopped(), never adapterReady(). Regression test: previously such an
 * adapter was skipped entirely (left running to silently become an untracked, zombie session), or
 * - before the first stopSession() fix - was added to _pendingReadyAdapters and deadlocked
 * adapterReady() just like the never-started case.
 */
void TestAdapterHub::stopSessionForceStopsMidHandshakeAdaptersWithoutWaiting()
{
    AdapterHub hub;
    auto* pActive = new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::Active, &hub);
    auto* pMidHandshake =
      new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::MidHandshake, &hub);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pActive);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pMidHandshake);

    QSignalSpy readySpy(&hub, &AdapterHub::adapterReady);

    hub.stopSession();

    /* Force-stopped, unlike the never-started case, to avoid leaving an untracked session. */
    QCOMPARE(pMidHandshake->stopSessionCalls(), 1);

    /* Not waited on: if it had been added to _pendingReadyAdapters, this would never fire,
       since a force-stopped mid-handshake adapter never emits adapterReady(). */
    hub.onManagerAdapterReady(QStringLiteral("iec104"));
    QCOMPARE(readySpy.count(), 1);
}

/*!
 * \brief stopSession() must purge a force-stopped mid-handshake adapter from
 * _pendingStartAdapters. Regression test: that adapter's force-stop only emits
 * sessionStopped(), never sessionStarted()/sessionError(), so nothing else would ever
 * clear its id - leaving _pendingStartAdapters unable to empty again and sessionStarted()
 * unable to ever re-fire, even once every other adapter starts successfully.
 */
void TestAdapterHub::stopSessionPurgesPendingStartForForceStoppedAdapters()
{
    AdapterHub hub;
    auto* pActive = new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::Active, &hub);
    auto* pMidHandshake =
      new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::MidHandshake, &hub);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pActive);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pMidHandshake);
    hub._pendingStartAdapters.insert(QStringLiteral("modbus"));

    QSignalSpy startedSpy(&hub, &AdapterHub::sessionStarted);

    hub.stopSession();

    /* If "modbus" were still pending, this alone would not be enough to empty the set. */
    hub.onManagerSessionStarted(QStringLiteral("iec104"));
    QCOMPARE(startedSpy.count(), 1);
}

/*!
 * \brief Regression test: stopSession() must not fire AdapterHub::adapterReady() until every
 * manager in the sweep has been asked to stop, even when one manager (a degraded session)
 * emits adapterReady() synchronously from within its own stopSession() call. Previously
 * _pendingReadyAdapters was populated one manager at a time, right before that manager's
 * stopSession() was called; a synchronous adapterReady() from an early manager (sorted first
 * by id) could drain the set and fire AdapterHub::adapterReady() before later managers had
 * even been asked to stop, and again a second time once they genuinely finished.
 */
void TestAdapterHub::stopSessionWaitsForAllAdaptersWhenOneStopsSynchronously()
{
    AdapterHub hub;
    auto* pSynchronous = new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::Active, &hub);
    pSynchronous->setEmitsAdapterReadySynchronouslyOnStop(true);
    auto* pAsynchronous = new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::Active, &hub);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pSynchronous);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pAsynchronous);
    /* Wire adapterReady() the same way initAdapter() does in production, so pSynchronous's
       synchronous emission from within stopSession() actually reaches AdapterHub. */
    hub.connectManager(pSynchronous, QStringLiteral("iec104"));
    hub.connectManager(pAsynchronous, QStringLiteral("modbus"));

    QSignalSpy readySpy(&hub, &AdapterHub::adapterReady);

    hub.stopSession();

    /* "iec104" already fired adapterReady() synchronously, but "modbus" has not yet acknowledged
       its own stop: AdapterHub::adapterReady() must not have fired yet. */
    QCOMPARE(readySpy.count(), 0);
    QCOMPARE(pAsynchronous->stopSessionCalls(), 1);

    emit pAsynchronous->adapterReady();
    QCOMPARE(readySpy.count(), 1);
}

/*!
 * \brief F13 regression test: a mixed idle/ready fleet must not deadlock the start path.
 *
 * Reproduces the exact scenario from the multi-device coherence audit: one adapter has
 * crashed back to IDLE while a sibling is still AWAITING_CONFIG. Under the old fleet-wide
 * isAdapterIdle()/isAdapterReady() predicates neither ever becomes true for this mix, so
 * initAdapter() was never called and adapterReady() never fired. initAdapter() must instead
 * restart only the idle manager and leave the ready one untouched, and the hub must still
 * become ready once that one manager reports back.
 */
void TestAdapterHub::initAdapterReinitializesOnlyIdleManagers()
{
    AdapterHub hub;
    auto* pIdle = new FakeAdapterManager(QStringLiteral("modbus"), FakeAdapterManager::FakeState::Idle, &hub);
    auto* pReady =
      new FakeAdapterManager(QStringLiteral("iec104"), FakeAdapterManager::FakeState::AwaitingConfig, &hub);
    hub._adapterManagers.insert(QStringLiteral("modbus"), pIdle);
    hub._adapterManagers.insert(QStringLiteral("iec104"), pReady);
    hub.connectManager(pIdle, QStringLiteral("modbus"));
    hub.connectManager(pReady, QStringLiteral("iec104"));

    QSignalSpy readySpy(&hub, &AdapterHub::adapterReady);

    hub.initAdapter();

    QCOMPARE(pIdle->initAdapterCalls(), 1);
    QCOMPARE(pReady->initAdapterCalls(), 0);

    /* Only the previously-idle adapter needs to report back - the already-ready sibling was
       never added to _pendingReadyAdapters, so it doesn't need to re-report. */
    hub.onManagerAdapterReady(QStringLiteral("modbus"));
    QCOMPARE(readySpy.count(), 1);
}

QTEST_GUILESS_MAIN(TestAdapterHub)
