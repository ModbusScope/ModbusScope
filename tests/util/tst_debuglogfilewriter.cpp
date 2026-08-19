
#include "tst_debuglogfilewriter.h"

#include "util/debuglogfilewriter.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

void TestDebugLogFileWriter::writeLineWhileDisabledDoesNothing()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString filePath = QDir(tmpDir.path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    writer.writeLine("should not be written");

    QVERIFY(!QFile::exists(filePath));
}

void TestDebugLogFileWriter::writeLineWhileEnabledAppendsLines()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString filePath = QDir(tmpDir.path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    QVERIFY(writer.setEnabled(true));
    writer.writeLine("foo");
    writer.writeLine("bar");
    QVERIFY(writer.setEnabled(false));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("foo\nbar\n"));
}

void TestDebugLogFileWriter::writtenLinesAreFlushedImmediately()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString filePath = QDir(tmpDir.path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    QVERIFY(writer.setEnabled(true));
    writer.writeLine("flushed");

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("flushed\n"));
}

void TestDebugLogFileWriter::reenablingAppendsRatherThanTruncates()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString filePath = QDir(tmpDir.path()).filePath("debug.log");

    DebugLogFileWriter firstWriter(filePath);
    QVERIFY(firstWriter.setEnabled(true));
    firstWriter.writeLine("first session");
    QVERIFY(firstWriter.setEnabled(false));

    DebugLogFileWriter secondWriter(filePath);
    QVERIFY(secondWriter.setEnabled(true));
    secondWriter.writeLine("second session");
    QVERIFY(secondWriter.setEnabled(false));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("first session\nsecond session\n"));
}

void TestDebugLogFileWriter::setEnabledFailsForUnwritablePath()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Parent directory does not exist, so QFile::open() must fail
    const QString filePath = QDir(tmpDir.path()).filePath("missing-subdir/debug.log");
    DebugLogFileWriter writer(filePath);

    QVERIFY(!writer.setEnabled(true));
    QVERIFY(!writer.isEnabled());

    writer.writeLine("should not be written");
    QVERIFY(!QFile::exists(filePath));
}

QTEST_GUILESS_MAIN(TestDebugLogFileWriter)
