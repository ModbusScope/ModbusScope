
#include "tst_recentfilemodule.h"

#include "util/recentfilemodule.h"

#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>

static const QString cTestOrg = QStringLiteral("ModbusScopeTest");
static const QString cTestApp = QStringLiteral("tst_recentfilemodule");

void TestRecentFileModule::init()
{
    QCoreApplication::setOrganizationName(cTestOrg);
    QCoreApplication::setApplicationName(cTestApp);
    QSettings().clear();
}

void TestRecentFileModule::cleanup()
{
    QSettings().clear();
}

void TestRecentFileModule::initiallyEmpty()
{
    RecentFileModule module;

    QVERIFY(module.recentProjectFiles().isEmpty());
}

void TestRecentFileModule::updateAddsFile()
{
    RecentFileModule module;

    module.updateRecentProjectFiles(QStringLiteral("/path/to/file.mbs"));

    const QList<QString> files = module.recentProjectFiles();
    QCOMPARE(files.size(), 1);
    QCOMPARE(files.first(), QStringLiteral("/path/to/file.mbs"));
}

void TestRecentFileModule::updateMovesExistingToFront()
{
    RecentFileModule module;

    module.updateRecentProjectFiles(QStringLiteral("/a.mbs"));
    module.updateRecentProjectFiles(QStringLiteral("/b.mbs"));
    module.updateRecentProjectFiles(QStringLiteral("/a.mbs"));

    const QList<QString> files = module.recentProjectFiles();
    QCOMPARE(files.size(), 2);
    QCOMPARE(files.first(), QStringLiteral("/a.mbs"));
}

void TestRecentFileModule::updateRespectsCap()
{
    RecentFileModule module;

    const int maxFiles = static_cast<int>(RecentFileModule::cMostRecentCount);
    for (int i = 0; i < maxFiles + 2; ++i)
    {
        module.updateRecentProjectFiles(QStringLiteral("/file%1.mbs").arg(i));
    }

    QCOMPARE(module.recentProjectFiles().size(), maxFiles);
}

void TestRecentFileModule::clearRecentProjectFiles()
{
    RecentFileModule module;

    module.updateRecentProjectFiles(QStringLiteral("/a.mbs"));
    module.updateRecentProjectFiles(QStringLiteral("/b.mbs"));

    module.clearRecentProjectFiles();

    QVERIFY(module.recentProjectFiles().isEmpty());
}

void TestRecentFileModule::persistsAcrossInstances()
{
    {
        RecentFileModule first;
        first.updateRecentProjectFiles(QStringLiteral("/a.mbs"));
        first.updateRecentProjectFiles(QStringLiteral("/b.mbs"));
        QSettings().sync();
    }

    RecentFileModule second;

    const QList<QString> files = second.recentProjectFiles();
    QCOMPARE(files.size(), 2);
    QCOMPARE(files.at(0), QStringLiteral("/b.mbs"));
    QCOMPARE(files.at(1), QStringLiteral("/a.mbs"));
}

void TestRecentFileModule::signalEmittedOnUpdate()
{
    RecentFileModule module;
    QSignalSpy spy(&module, &RecentFileModule::mostRecentProjectFileUpdated);
    QVERIFY(spy.isValid());

    module.updateRecentProjectFiles(QStringLiteral("/a.mbs"));

    QCOMPARE(spy.count(), 1);
}

void TestRecentFileModule::signalEmittedOnClear()
{
    RecentFileModule module;
    module.updateRecentProjectFiles(QStringLiteral("/a.mbs"));

    QSignalSpy spy(&module, &RecentFileModule::mostRecentProjectFileUpdated);
    QVERIFY(spy.isValid());
    module.clearRecentProjectFiles();

    QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(TestRecentFileModule)
