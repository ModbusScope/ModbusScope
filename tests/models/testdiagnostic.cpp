
#include <QtTest/QtTest>

#include "testdiagnostic.h"

#include "diagnostic.h"

void TestDiagnostic::init()
{

}

void TestDiagnostic::cleanup()
{

}

void TestDiagnostic::fullConstructor()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString("Test"));
    QCOMPARE(log.category(), QStringLiteral("scope.comm"));
    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);
    QCOMPARE(log.message(), QString("Test"));
    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_WARNING, now, QString(""));
    QCOMPARE(log.category(), QStringLiteral("scope.comm"));
    QCOMPARE(log.severity(), Diagnostic::LOG_WARNING);
    QCOMPARE(log.message(), QString(""));
    QCOMPARE(log.timestamp(), now);
}

void TestDiagnostic::setSeverity()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);

    log.setSeverity(Diagnostic::LOG_WARNING);
    QCOMPARE(log.severity(), Diagnostic::LOG_WARNING);

    log.setSeverity(Diagnostic::LOG_DEBUG);
    QCOMPARE(log.severity(), Diagnostic::LOG_DEBUG);
}

void TestDiagnostic::setCategory()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.category(), QStringLiteral("scope.comm"));

    log.setCategory(QStringLiteral("test"));
    QCOMPARE(log.category(), QStringLiteral("test"));
}

void TestDiagnostic::setTimeStamp()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log.setTimestamp(now);
    QCOMPARE(log.timestamp(), now);
}

void TestDiagnostic::setMessage()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.message(), QString(""));

    QString newMsg = QString("NewMsg");

    log.setMessage(newMsg);
    QCOMPARE(log.message(), newMsg);
}

void TestDiagnostic::severityString()
{
    Diagnostic log = Diagnostic(QStringLiteral("scope.comm"), Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severityString(), QString("Info"));

    log.setSeverity(Diagnostic::LOG_WARNING);
    QCOMPARE(log.severityString(), QString("Warning"));

    log.setSeverity(Diagnostic::LOG_DEBUG);
    QCOMPARE(log.severityString(), QString("Debug"));

    log.setSeverity(static_cast<Diagnostic::LogSeverity>(99));
    QCOMPARE(log.severityString(), QString("Unknown"));
}

QTEST_GUILESS_MAIN(TestDiagnostic)
