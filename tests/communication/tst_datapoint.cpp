
#include "tst_datapoint.h"

#include "communication/datapoint.h"
#include "models/device.h"

#include <QTest>

void TestDataPoint::init()
{
}

void TestDataPoint::cleanup()
{
}

void TestDataPoint::defaultConstructor()
{
    DataPoint dp;

    QCOMPARE(dp.address(), QStringLiteral(""));
    QCOMPARE(dp.deviceId(), Device::cFirstDeviceId);
}

void TestDataPoint::constructorWithArgs()
{
    DataPoint dp(QStringLiteral("40001"), 2u);

    QCOMPARE(dp.address(), QStringLiteral("40001"));
    QCOMPARE(dp.deviceId(), 2u);
}

void TestDataPoint::copyConstructor()
{
    DataPoint original(QStringLiteral("40001"), 3u);
    DataPoint copy(original);

    QCOMPARE(copy.address(), original.address());
    QCOMPARE(copy.deviceId(), original.deviceId());
}

void TestDataPoint::assignmentOperator()
{
    DataPoint a(QStringLiteral("40001"), 1u);
    DataPoint b(QStringLiteral("40002"), 2u);

    b = a;

    QCOMPARE(b.address(), QStringLiteral("40001"));
    QCOMPARE(b.deviceId(), 1u);
}

void TestDataPoint::selfAssignment()
{
    DataPoint dp(QStringLiteral("40001"), 1u);

    // self-assignment must not corrupt state
    dp = dp;

    QCOMPARE(dp.address(), QStringLiteral("40001"));
    QCOMPARE(dp.deviceId(), 1u);
}

void TestDataPoint::equalityOperator()
{
    DataPoint a(QStringLiteral("40001"), 1u);
    DataPoint b(QStringLiteral("40001"), 1u);
    DataPoint differentAddress(QStringLiteral("40002"), 1u);
    DataPoint differentDevice(QStringLiteral("40001"), 2u);

    QVERIFY(a == b);
    QVERIFY(!(a == differentAddress));
    QVERIFY(!(a == differentDevice));
}

void TestDataPoint::description()
{
    DataPoint dp(QStringLiteral("40001"), 2u);
    const QString desc = dp.description();

    QVERIFY(desc.contains(QStringLiteral("40001")));
    QVERIFY(desc.contains(QStringLiteral("2")));
}

void TestDataPoint::dumpListToString()
{
    QList<DataPoint> list;
    list.append(DataPoint(QStringLiteral("40001"), 1u));
    list.append(DataPoint(QStringLiteral("40002"), 1u));

    const QString result = DataPoint::dumpListToString(list);

    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains(QStringLiteral("40001")));
    QVERIFY(result.contains(QStringLiteral("40002")));
}

void TestDataPoint::dumpEmptyListToString()
{
    const QList<DataPoint> empty;
    const QString result = DataPoint::dumpListToString(empty);

    QVERIFY(!result.isNull());
}

QTEST_GUILESS_MAIN(TestDataPoint)
