
#include <QtTest/QtTest>

#include "testerrorlog.h"

#include "errorlog.h"

void TestErrorLog::init()
{

}

void TestErrorLog::cleanup()
{

}

void TestErrorLog::fullConstructor()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString("Test"));
    QCOMPARE(log.severity(), ErrorLog::LOG_INFO);
    QCOMPARE(log.message(), QString("Test"));
    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log = ErrorLog(ErrorLog::LOG_ERROR, now, QString(""));
    QCOMPARE(log.severity(), ErrorLog::LOG_ERROR);
    QCOMPARE(log.message(), QString(""));
    QCOMPARE(log.timestamp(), now);
}

void TestErrorLog::setSeverity()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severity(), ErrorLog::LOG_INFO);

    log.setSeverity(ErrorLog::LOG_ERROR);
    QCOMPARE(log.severity(), ErrorLog::LOG_ERROR);
}

void TestErrorLog::setTimeStamp()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log.setTimestamp(now);
    QCOMPARE(log.timestamp(), now);
}

void TestErrorLog::setMessage()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.message(), QString(""));

    QString newMsg = QString("NewMsg");

    log.setMessage(newMsg);
    QCOMPARE(log.message(), newMsg);
}

void TestErrorLog::categoryString()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.severityString(), QString("Info"));

    log.setSeverity(ErrorLog::LOG_ERROR);
    QCOMPARE(log.severityString(), QString("Error"));

    log.setSeverity(static_cast<ErrorLog::LogSeverity>(99));
    QCOMPARE(log.severityString(), QString("Unknown"));
}

QTEST_GUILESS_MAIN(TestErrorLog)
