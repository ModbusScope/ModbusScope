#include "tst_adapterpoll.h"

#include "ProtocolAdapter/adapterhub.h"
#include "communication/adapterpoll.h"
#include "communication/datapoint.h"
#include "models/settingsmodel.h"

#include <QSignalSpy>
#include <QTest>

/* ---- Mock AdapterHub ---- */

class MockAdapterHub : public AdapterHub
{
    Q_OBJECT
public:
    explicit MockAdapterHub(QObject* parent = nullptr) : AdapterHub(parent)
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
    void startSession(const QString& adapterId, const QStringList& expressions) override
    {
        _startCalls.append({ adapterId, expressions });
    }
    void initAdapter() override
    {
        _initCount++;
    }
    void stopSession() override
    {
        _stopCount++;
    }
    void requestReadData() override
    {
    }

    void triggerAdapterReady()
    {
        emit adapterReady();
    }
    void triggerReadDataResult(const QString& adapterId, ResultDoubleList results)
    {
        emit readDataResult(adapterId, results);
    }

    bool _mockReady{ false };
    bool _mockIdle{ false };
    QList<QPair<QString, QStringList>> _startCalls;
    int _initCount{ 0 };
    int _stopCount{ 0 };
};

/* ---- Fixture ---- */

static SettingsModel* s_pSettingsModel = nullptr;
static MockAdapterHub* s_pMockHub = nullptr;
static AdapterPoll* s_pPoll = nullptr;

void TestAdapterPoll::init()
{
    s_pSettingsModel = new SettingsModel;
    s_pMockHub = new MockAdapterHub;
    s_pPoll = new AdapterPoll(s_pSettingsModel, s_pMockHub);
}

void TestAdapterPoll::cleanup()
{
    delete s_pPoll;
    delete s_pMockHub;
    delete s_pSettingsModel;
}

/* ---- Tests ---- */

void TestAdapterPoll::startCommunicationWhenAdapterReady()
{
    s_pMockHub->_mockReady = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);

    QCOMPARE(s_pMockHub->_startCalls.size(), 1);
    QCOMPARE(s_pMockHub->_initCount, 0);
    QVERIFY(s_pPoll->isActive());
}

void TestAdapterPoll::startCommunicationWhenAdapterIdle()
{
    s_pMockHub->_mockReady = false;
    s_pMockHub->_mockIdle = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);

    /* startSession not called yet — waiting for adapterReady */
    QCOMPARE(s_pMockHub->_startCalls.size(), 0);
    /* initAdapter called to kick off the process */
    QCOMPARE(s_pMockHub->_initCount, 1);

    s_pMockHub->triggerAdapterReady();

    QCOMPARE(s_pMockHub->_startCalls.size(), 1);
    QCOMPARE(s_pMockHub->_startCalls[0].first, QStringLiteral("modbus"));
    QCOMPARE(s_pMockHub->_startCalls[0].second, QStringList{ QStringLiteral("${h0}") });
}

void TestAdapterPoll::startCommunicationWhenAdapterInitializing()
{
    s_pMockHub->_mockReady = false;
    s_pMockHub->_mockIdle = false;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h1}"), 1) };
    s_pPoll->startCommunication(registers);

    /* Neither startSession nor initAdapter called — adapter is mid-init */
    QCOMPARE(s_pMockHub->_startCalls.size(), 0);
    QCOMPARE(s_pMockHub->_initCount, 0);

    s_pMockHub->triggerAdapterReady();

    QCOMPARE(s_pMockHub->_startCalls.size(), 1);
    QCOMPARE(s_pMockHub->_startCalls[0].first, QStringLiteral("modbus"));
    QCOMPARE(s_pMockHub->_startCalls[0].second, QStringList{ QStringLiteral("${h1}") });
}

void TestAdapterPoll::doubleStartCommunicationWhileInitializing()
{
    s_pMockHub->_mockReady = false;
    s_pMockHub->_mockIdle = false;

    QList<DataPoint> registers1{ DataPoint(QStringLiteral("${h0}"), 1) };
    QList<DataPoint> registers2{ DataPoint(QStringLiteral("${h1}"), 1) };

    s_pPoll->startCommunication(registers1);
    /* Second call before adapterReady fires — must not register a second connection */
    s_pPoll->startCommunication(registers2);

    s_pMockHub->triggerAdapterReady();

    /* startSession called exactly once with the latest expressions */
    QCOMPARE(s_pMockHub->_startCalls.size(), 1);
    QCOMPARE(s_pMockHub->_startCalls[0].second, QStringList{ QStringLiteral("${h1}") });
}

void TestAdapterPoll::stopCommunicationClearsPendingState()
{
    s_pMockHub->_mockReady = false;
    s_pMockHub->_mockIdle = true;

    QList<DataPoint> registers{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers);
    QVERIFY(s_pPoll->isActive());

    s_pPoll->stopCommunication();
    QVERIFY(!s_pPoll->isActive());
    QCOMPARE(s_pMockHub->_stopCount, 1);

    /* adapterReady fires after stop — must not trigger startSession */
    s_pMockHub->triggerAdapterReady();
    QCOMPARE(s_pMockHub->_startCalls.size(), 0);
}

void TestAdapterPoll::stopCommunicationAllowsNewWaitAfterRestart()
{
    s_pMockHub->_mockReady = false;
    s_pMockHub->_mockIdle = false;

    QList<DataPoint> registers1{ DataPoint(QStringLiteral("${h0}"), 1) };
    s_pPoll->startCommunication(registers1);

    /* Stop before adapterReady fires */
    s_pPoll->stopCommunication();

    /* After stop, a new startCommunication should work — one-shot slot must be re-registerable */
    QList<DataPoint> registers2{ DataPoint(QStringLiteral("${h2}"), 1) };
    s_pPoll->startCommunication(registers2);

    s_pMockHub->triggerAdapterReady();

    QCOMPARE(s_pMockHub->_startCalls.size(), 1);
    QCOMPARE(s_pMockHub->_startCalls[0].second, QStringList{ QStringLiteral("${h2}") });
}

QTEST_GUILESS_MAIN(TestAdapterPoll)

#include "tst_adapterpoll.moc"
