#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "../src/models/errorlog.h"

using namespace testing;

TEST(ErrorLog, fullConstructor)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString("Test"));
    EXPECT_EQ(log.category(), ErrorLog::LOG_INFO);
    EXPECT_EQ(log.message(), QString("Test"));
    EXPECT_EQ(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log = ErrorLog(ErrorLog::LOG_ERROR, now, QString(""));
    EXPECT_EQ(log.category(), ErrorLog::LOG_ERROR);
    EXPECT_EQ(log.message(), QString(""));
    EXPECT_EQ(log.timestamp(), now);
}

TEST(ErrorLog, setCategory)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    EXPECT_EQ(log.category(), ErrorLog::LOG_INFO);

    log.setCategory(ErrorLog::LOG_ERROR);
    EXPECT_EQ(log.category(), ErrorLog::LOG_ERROR);
}

TEST(ErrorLog, setTimeStamp)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    EXPECT_EQ(log.timestamp(), QDateTime());

    QDateTime now = QDateTime::currentDateTime();
    log.setTimestamp(now);
    EXPECT_EQ(log.timestamp(), now);
}

TEST(ErrorLog, setMessage)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    EXPECT_EQ(log.message(), QString(""));

    QString newMsg = QString("NewMsg");

    log.setMessage(newMsg);
    EXPECT_EQ(log.message(), newMsg);
}

TEST(ErrorLog, categoryString)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), QString(""));

    EXPECT_EQ(log.categoryString(), QString("Info"));

    log.setCategory(ErrorLog::LOG_ERROR);
    EXPECT_EQ(log.categoryString(), QString("Error"));

    log.setCategory(static_cast<ErrorLog::LogCategory>(99));
    EXPECT_EQ(log.categoryString(), QString("Unknown"));
}
