
#include <QtTest/QtTest>

#include "formatrelativetime.h"

#include "testformatrelativetime.h"

#define ADD_TEST(ticks, result)      QTest::newRow(result) << static_cast<qint64>(ticks) << result


void TestFormatRelativeTime::init()
{

}

void TestFormatRelativeTime::cleanup()
{

}

void TestFormatRelativeTime::formatTimeSmallScale_data()
{
    QTest::addColumn<qint64>("ticks");
    QTest::addColumn<QString>("result");

    ADD_TEST(100, "100");
    ADD_TEST(1000, "1000");
    ADD_TEST(10000, "10000");
    ADD_TEST(100000, "100000");

    ADD_TEST(-100, "-100");
    ADD_TEST(-10000, "-10000");
}

void TestFormatRelativeTime::formatTimeSmallScale()
{
    QFETCH(qint64, ticks);
    QFETCH(QString, result);

    QString str = FormatRelativeTime::formatTimeSmallScale(ticks);

    QCOMPARE(str, result);
}

void TestFormatRelativeTime::formatTime_data()
{
    QTest::addColumn<qint64>("ticks");
    QTest::addColumn<QString>("result");

    ADD_TEST(5,             "00:00:00,005");
    ADD_TEST(500,           "00:00:00,500");
    ADD_TEST(5000,          "00:00:05,000");
    ADD_TEST(50*60*1000,    "00:50:00,000");
    ADD_TEST(60*60*1000,    "01:00:00,000");

    ADD_TEST(-5,             "-00:00:00,005");
    ADD_TEST(-500,           "-00:00:00,500");
    ADD_TEST(-5000,          "-00:00:05,000");
    ADD_TEST(-50*60*1000,    "-00:50:00,000");
    ADD_TEST(-60*60*1000,    "-01:00:00,000");
}

void TestFormatRelativeTime::formatTime()
{
    QFETCH(qint64, ticks);
    QFETCH(QString, result);

    syncDecimalPoint(result);

    QString str = FormatRelativeTime::formatTime(ticks);

    QCOMPARE(str, result);
}

void TestFormatRelativeTime::formatTime_Absolute_data()
{
    QTest::addColumn<qint64>("ticks");
    QTest::addColumn<QString>("result");

    ADD_TEST(1602935825011,      "17/10/2020\n13:57:05,011");
    ADD_TEST(1602899825011,      "17/10/2020\n03:57:05,011");
}

void TestFormatRelativeTime::formatTime_Absolute()
{
    QFETCH(qint64, ticks);
    QFETCH(QString, result);

    syncDecimalPoint(result);

    QString str = FormatRelativeTime::formatTime(ticks);

    QCOMPARE(str, result);
}

void TestFormatRelativeTime::syncDecimalPoint(QString &timeStr)
{
    QChar decimalPoint = ',';

    /* Replace decimal point if needed */
    if (QLocale::system().decimalPoint() != decimalPoint)
    {
        timeStr = timeStr.replace(decimalPoint, QLocale::system().decimalPoint());
    }
}

QTEST_GUILESS_MAIN(TestFormatRelativeTime)
