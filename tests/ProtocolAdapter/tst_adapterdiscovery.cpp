#include "tst_adapterdiscovery.h"

#include "ProtocolAdapter/adapterdiscovery.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

static bool touchExecutable(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
    {
        return false;
    }
    f.close();
#ifndef Q_OS_WIN
    return f.setPermissions(f.permissions() | QFileDevice::ExeOwner);
#else
    return true;
#endif
}

#ifdef Q_OS_WIN
static QString adapterName(const QString& base)
{
    return base + QStringLiteral(".exe");
}
#else
static QString adapterName(const QString& base)
{
    return base;
}
#endif

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
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(adapterName(QStringLiteral("modbusadapter")))));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].id, QStringLiteral("modbus"));
    QVERIFY(result[0].binaryPath.contains(QStringLiteral("modbusadapter")));
}

void TestAdapterDiscovery::discoverMultipleAdapters()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(adapterName(QStringLiteral("modbusadapter")))));
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(adapterName(QStringLiteral("canopenadapter")))));

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
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(adapterName(QStringLiteral("modbusadapter")))));
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(QStringLiteral("modbusscope"))));
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(QStringLiteral("readme.txt"))));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].id, QStringLiteral("modbus"));
}

void TestAdapterDiscovery::ignoreEmptyIdAdapter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    /* A file named exactly "adapter" would yield an empty id — must be ignored */
    QVERIFY(touchExecutable(QDir(dir.path()).filePath(adapterName(QStringLiteral("adapter")))));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QVERIFY(result.isEmpty());
}

void TestAdapterDiscovery::ignoreNonExecutableAdapter()
{
#ifdef Q_OS_WIN
    QSKIP("Execute-bit check is Unix-only; Windows uses .exe extension instead");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    /* Create a correctly-named file but without the execute bit — must be ignored */
    QFile f(QDir(dir.path()).filePath(QStringLiteral("modbusadapter")));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.close();
    f.setPermissions(f.permissions() & ~(QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther));

    const QList<AdapterInfo> result = AdapterDiscovery::discover(dir.path());
    QVERIFY(result.isEmpty());
#endif
}

QTEST_GUILESS_MAIN(TestAdapterDiscovery)

#include "tst_adapterdiscovery.moc"
