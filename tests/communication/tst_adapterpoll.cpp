#include "tst_adapterpoll.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "communication/adapterpoll.h"
#include "communication/datapoint.h"
#include "models/settingsmodel.h"

#include <QSignalSpy>
#include <QTest>

/* ---- Mock AdapterManager ---- */

class MockAdapterManager : public AdapterManager
{
    Q_OBJECT
public:
    explicit MockAdapterManager(QObject* parent = nullptr) : AdapterManager(nullptr, parent)
    {
    }

    bool isAdapterReady() const override
    {
        return _mockReady;
    }
    bool isAdapterIdle() const override
    {
        return _mockIdle;
    }
    void startSession(const QStringList& expressions) override
    {
        _startCalls.append(expressions);
    }
    void initAdapter() override
    {
        _initCount++;
    }
    void stopSession() override
    {
        _stopCount++;
    }

    void triggerAdapterReady()
    {
        emit adapterReady();
    }

    bool _mockReady{ false };
    bool _mockIdle{ false };
    QList<QStringList> _startCalls;
    int _initCount{ 0 };
    int _stopCount{ 0 };
};

/* ---- Fixture ---- */

static SettingsModel* s_pSettingsModel = nullptr;
static MockAdapterManager* s_pMockManager = nullptr;
static AdapterPoll* s_pPoll = nullptr;

void TestAdapterPoll::init()
{
    s_pSettingsModel = new SettingsModel;
    s_pMockManager = new MockAdapterManager;
    s_pPoll = new AdapterPoll(s_pSettingsModel, s_pMockManager);
}

void TestAdapterPoll::cleanup()
{
    delete s_pPoll;
    delete s_pMockManager;
    delete s_pSettingsModel;
}

/* ---- Tests ---- */

void TestAdapterPoll::startCommunicationWhenAdapterReady()
{
    s_pMockManager->_mockReady = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);

    QCOMPARE(s_pMockManager->_startCalls.size(), 1);
    QCOMPARE(s_pMockManager->_initCount, 0);
    QVERIFY(s_pPoll->isActive());
}

void TestAdapterPoll::startCommunicationWhenAdapterIdle()
{
    s_pMockManager->_mockReady = false;
    s_pMockManager->_mockIdle = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);

    /* startSession not called yet — waiting for adapterReady */
    QCOMPARE(s_pMockManager->_startCalls.size(), 0);
    /* initAdapter called to kick off the process */
    QCOMPARE(s_pMockManager->_initCount, 1);

    s_pMockManager->triggerAdapterReady();

    QCOMPARE(s_pMockManager->_startCalls.size(), 1);
    QCOMPARE(s_pMockManager->_startCalls[0], QStringList{ QStringLiteral("${h0}") });
}

void TestAdapterPoll::startCommunicationWhenAdapterInitializing()
{
    s_pMockManager->_mockReady = false;
    s_pMockManager->_mockIdle = false;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h1}"), 1) };
    s_pPoll->startCommunication(registers);

    /* Neither startSession nor initAdapter called — adapter is mid-init */
    QCOMPARE(s_pMockManager->_startCalls.size(), 0);
    QCOMPARE(s_pMockManager->_initCount, 0);

    s_pMockManager->triggerAdapterReady();

    QCOMPARE(s_pMockManager->_startCalls.size(), 1);
    QCOMPARE(s_pMockManager->_startCalls[0], QStringList{ QStringLiteral("${h1}") });
}

void TestAdapterPoll::doubleStartCommunicationWhileInitializing()
{
    s_pMockManager->_mockReady = false;
    s_pMockManager->_mockIdle = false;

    QList<DataPoint> registers1{ DataPoint(QStringLiteral("${h0}"), 1) };
    QList<DataPoint> registers2{ DataPoint(QStringLiteral("${h1}"), 1) };

    s_pPoll->startCommunication(registers1);
    /* Second call before adapterReady fires — must not register a second connection */
    s_pPoll->startCommunication(registers2);

    s_pMockManager->triggerAdapterReady();

    /* startSession called exactly once with the latest expressions */
    QCOMPARE(s_pMockManager->_startCalls.size(), 1);
    QCOMPARE(s_pMockManager->_startCalls[0], QStringList{ QStringLiteral("${h1}") });
}

void TestAdapterPoll::stopCommunicationClearsPendingState()
{
    s_pMockManager->_mockReady = false;
    s_pMockManager->_mockIdle = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);
    QVERIFY(s_pPoll->isActive());

    s_pPoll->stopCommunication();
    QVERIFY(!s_pPoll->isActive());
    QCOMPARE(s_pMockManager->_stopCount, 1);

    /* adapterReady fires after stop — must not trigger startSession */
    s_pMockManager->triggerAdapterReady();
    QCOMPARE(s_pMockManager->_startCalls.size(), 0);
}

void TestAdapterPoll::stopCommunicationAllowsNewWaitAfterRestart()
{
    s_pMockManager->_mockReady = false;
    s_pMockManager->_mockIdle = false;

    QList<DataPoint> registers1{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers1);

    /* Stop before adapterReady fires */
    s_pPoll->stopCommunication();

    /* After stop, a new startCommunication should work — one-shot slot must be re-registerable */
    QList<DataPoint> registers2{ DataPoint(QStringLiteral("${h2}"), 1) };
    s_pPoll->startCommunication(registers2);

    s_pMockManager->triggerAdapterReady();

    QCOMPARE(s_pMockManager->_startCalls.size(), 1);
    QCOMPARE(s_pMockManager->_startCalls[0], QStringList{ QStringLiteral("${h2}") });
}

QTEST_GUILESS_MAIN(TestAdapterPoll)

#include "tst_adapterpoll.moc"
