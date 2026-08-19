
#include "tst_debuglogfilewriter.h"

#include "util/debuglogfilewriter.h"

#include <QDir>
#include <QFile>
#include <QTest>

void TestDebugLogFileWriter::init()
{
    _pTempDir = new QTemporaryDir();
    QVERIFY(_pTempDir->isValid());
}

void TestDebugLogFileWriter::cleanup()
{
    delete _pTempDir;
}

void TestDebugLogFileWriter::writeLineWhileDisabledDoesNothing()
{
    const QString filePath = QDir(_pTempDir->path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    writer.writeLine("should not be written");

    QVERIFY(!QFile::exists(filePath));
}

void TestDebugLogFileWriter::writeLineWhileEnabledAppendsLines()
{
    const QString filePath = QDir(_pTempDir->path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    writer.setEnabled(true);
    writer.writeLine("foo");
    writer.writeLine("bar");
    writer.setEnabled(false);

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("foo\nbar\n"));
}

void TestDebugLogFileWriter::writtenLinesAreFlushedImmediately()
{
    const QString filePath = QDir(_pTempDir->path()).filePath("debug.log");
    DebugLogFileWriter writer(filePath);

    writer.setEnabled(true);
    writer.writeLine("flushed");

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("flushed\n"));
}

void TestDebugLogFileWriter::reenablingAppendsRatherThanTruncates()
{
    const QString filePath = QDir(_pTempDir->path()).filePath("debug.log");

    DebugLogFileWriter firstWriter(filePath);
    firstWriter.setEnabled(true);
    firstWriter.writeLine("first session");
    firstWriter.setEnabled(false);

    DebugLogFileWriter secondWriter(filePath);
    secondWriter.setEnabled(true);
    secondWriter.writeLine("second session");
    secondWriter.setEnabled(false);

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString(file.readAll()), QStringLiteral("first session\nsecond session\n"));
}

QTEST_GUILESS_MAIN(TestDebugLogFileWriter)
