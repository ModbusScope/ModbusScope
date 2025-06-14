
#include "tst_diagnostic.h"

#include "models/diagnostic.h"

#include <QTest>

void TestDiagnostic::init()
{

}

void TestDiagnostic::cleanup()
{

}

void TestDiagnostic::fullConstructor()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 10, QString("Test"));
    QCOMPARE(log.category(), QStringLiteral("scope.comm"));
    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);
    QCOMPARE(log.message(), QString("Test"));
    QCOMPARE(log.timeOffset(), 10);

    log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_WARNING, 100, QString(""));
    QCOMPARE(log.category(), QStringLiteral("scope.comm"));
    QCOMPARE(log.severity(), Diagnostic::LOG_WARNING);
    QCOMPARE(log.message(), QString(""));
    QCOMPARE(log.timeOffset(), 100);
}

void TestDiagnostic::setSeverity()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 0, QString(""));

    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);

    log.setSeverity(Diagnostic::LOG_WARNING);
    QCOMPARE(log.severity(), Diagnostic::LOG_WARNING);

    log.setSeverity(Diagnostic::LOG_DEBUG);
    QCOMPARE(log.severity(), Diagnostic::LOG_DEBUG);
}

void TestDiagnostic::setCategory()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 20, QString(""));

    QCOMPARE(log.category(), QStringLiteral("scope.comm"));

    log.setCategory(QStringLiteral("test"));
    QCOMPARE(log.category(), QStringLiteral("test"));
}

void TestDiagnostic::setTimeStamp()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 10, QString(""));

    QCOMPARE(log.timeOffset(), 10);

    log.setTimeOffset(100);
    QCOMPARE(log.timeOffset(), 100);
}

void TestDiagnostic::setMessage()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 0, QString(""));

    QCOMPARE(log.message(), QString(""));

    QString newMsg = QString("NewMsg");

    log.setMessage(newMsg);
    QCOMPARE(log.message(), newMsg);
}

void TestDiagnostic::severityString()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 0, QString(""));

    QCOMPARE(log.severityString(), QString("INFO"));

    log.setSeverity(Diagnostic::LOG_WARNING);
    QCOMPARE(log.severityString(), QString("WARNING"));

    log.setSeverity(Diagnostic::LOG_DEBUG);
    QCOMPARE(log.severityString(), QString("DEBUG"));

    log.setSeverity(static_cast<Diagnostic::LogSeverity>(99));
    QCOMPARE(log.severityString(), QString("UNKNOWN"));
}

void TestDiagnostic::toString_1()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 100, QString("Test"));

    QString result = QStringLiteral("00000100 - INFO [scope.comm]: Test");

    QCOMPARE(log.toString(), result);
}

void TestDiagnostic::toString_2()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 99999999, QString("Test"));

    QString result = QStringLiteral("99999999 - INFO [scope.comm]: Test");

    QCOMPARE(log.toString(), result);
}

void TestDiagnostic::toExportString_1()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 100, QString("Test"));

    QString result = QStringLiteral("00000100\tINFO\tscope.comm\tTest");

    QCOMPARE(log.toExportString(), result);
}

void TestDiagnostic::toExportString_2()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, 99999999, QString("Test"));

    QString result = QStringLiteral("99999999\tINFO\tscope.comm\tTest");

    QCOMPARE(log.toExportString(), result);
}


QTEST_GUILESS_MAIN(TestDiagnostic)
