#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "src/models/errorlog.h"

using namespace testing;

TEST(TestCase, TestSet)
{
    ErrorLog log = ErrorLog(ErrorLog::LOG_INFO, QDateTime(), "Test");
    EXPECT_EQ(log.category(), ErrorLog::LOG_INFO);
    EXPECT_EQ(log.category(), ErrorLog::LOG_ERROR);
}
