
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
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString("Test"));
    QCOMPARE(log.category(), Diagnostic::LOG_COMMUNICATION);
    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);
    QCOMPARE(log.message(), QString("Test"));
    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_ERROR, now, QString(""));
    QCOMPARE(log.category(), Diagnostic::LOG_COMMUNICATION);
    QCOMPARE(log.severity(), Diagnostic::LOG_ERROR);
    QCOMPARE(log.message(), QString(""));
    QCOMPARE(log.timestamp(), now);
}

void TestDiagnostic::setSeverity()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severity(), Diagnostic::LOG_INFO);

    log.setSeverity(Diagnostic::LOG_ERROR);
    QCOMPARE(log.severity(), Diagnostic::LOG_ERROR);
}

void TestDiagnostic::setCategory()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.category(), Diagnostic::LOG_COMMUNICATION);

    log.setCategory(static_cast<Diagnostic::LogCategory>(99));
    QCOMPARE(log.category(), static_cast<Diagnostic::LogCategory>(99));
}

void TestDiagnostic::setTimeStamp()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log.setTimestamp(now);
    QCOMPARE(log.timestamp(), now);
}

void TestDiagnostic::setMessage()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.message(), QString(""));

    QString newMsg = QString("NewMsg");

    log.setMessage(newMsg);
    QCOMPARE(log.message(), newMsg);
}

void TestDiagnostic::severityString()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severityString(), QString("Info"));

    log.setSeverity(Diagnostic::LOG_ERROR);
    QCOMPARE(log.severityString(), QString("Error"));

    log.setSeverity(static_cast<Diagnostic::LogSeverity>(99));
    QCOMPARE(log.severityString(), QString("Unknown"));
}

void TestDiagnostic::categoryString()
{
    Diagnostic log = Diagnostic(Diagnostic::LOG_COMMUNICATION, Diagnostic::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.categoryString(), QString("Communication"));

    log.setCategory(static_cast<Diagnostic::LogCategory>(99));
    QCOMPARE(log.categoryString(), QString("Unknown"));
}

QTEST_GUILESS_MAIN(TestDiagnostic)
