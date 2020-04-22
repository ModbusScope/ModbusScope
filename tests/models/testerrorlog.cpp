
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
    QCOMPARE(log.category(), ErrorLog::LOG_INFO);
    QCOMPARE(log.message(), QString("Test"));
    QCOMPARE(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log = ErrorLog(ErrorLog::LOG_ERROR, now, QString(""));
    QCOMPARE(log.category(), ErrorLog::LOG_ERROR);
    QCOMPARE(log.message(), QString(""));
    QCOMPARE(log.timestamp(), now);
}

void TestErrorLog::setCategory()
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    QCOMPARE(log.category(), ErrorLog::LOG_INFO);

    log.setCategory(ErrorLog::LOG_ERROR);
    QCOMPARE(log.category(), ErrorLog::LOG_ERROR);
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

    QCOMPARE(log.categoryString(), QString("Info"));

    log.setCategory(ErrorLog::LOG_ERROR);
    QCOMPARE(log.categoryString(), QString("Error"));

    log.setCategory(static_cast<ErrorLog::LogCategory>(99));
    QCOMPARE(log.categoryString(), QString("Unknown"));
}

QTEST_GUILESS_MAIN(TestErrorLog)
