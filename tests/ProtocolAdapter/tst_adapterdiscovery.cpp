#include "tst_adapterdiscovery.h"

#include "ProtocolAdapter/adapterdiscovery.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

static void touchFile(const QString& path)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly);
}

void TestAdapterDiscovery::discoverEmpty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QVERIFY(result.isEmpty());
}

void TestAdapterDiscovery::discoverSingleAdapter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    touchFile(QDir(dir.path()).filePath(QStringLiteral("modbusadapter")));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].id, QStringLiteral("modbus"));
    QVERIFY(result[0].binaryPath.endsWith(QStringLiteral("modbusadapter")));
}

void TestAdapterDiscovery::discoverMultipleAdapters()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    touchFile(QDir(dir.path()).filePath(QStringLiteral("modbusadapter")));
    touchFile(QDir(dir.path()).filePath(QStringLiteral("canopenadapter")));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QCOMPARE(result.size(), 2);

    QStringList ids;
    for (const AdapterInfo& info : result)
    {
        ids.append(info.id);
    }
    ids.sort();
    QCOMPARE(ids[0], QStringLiteral("canopen"));
    QCOMPARE(ids[1], QStringLiteral("modbus"));
}

void TestAdapterDiscovery::ignoreNonAdapterFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    touchFile(QDir(dir.path()).filePath(QStringLiteral("modbusadapter")));
    touchFile(QDir(dir.path()).filePath(QStringLiteral("modbusscope")));
    touchFile(QDir(dir.path()).filePath(QStringLiteral("readme.txt")));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].id, QStringLiteral("modbus"));
}

void TestAdapterDiscovery::ignoreEmptyIdAdapter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    /* A file named exactly "adapter" would yield an empty id — must be ignored */
    touchFile(QDir(dir.path()).filePath(QStringLiteral("adapter")));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QVERIFY(result.isEmpty());
}

QTEST_GUILESS_MAIN(TestAdapterDiscovery)

#include "tst_adapterdiscovery.moc"
