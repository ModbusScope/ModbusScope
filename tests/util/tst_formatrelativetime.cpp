
#include <QtTest/QtTest>

#include "formatrelativetime.h"

#include "tst_formatrelativetime.h"

#define ADD_TEST(ticks, result)      QTest::newRow(result) << static_cast<qint64>(ticks) << result


void TestFormatRelativeTime::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

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

    /* Positive numbers */
    ADD_TEST(0,             "00:00:00,000");
    ADD_TEST(5,             "00:00:00,005");
    ADD_TEST(500,           "00:00:00,500");
    ADD_TEST(5000,          "00:00:05,000");
    ADD_TEST(50*60*1000,    "00:50:00,000");
    ADD_TEST(60*60*1000,    "01:00:00,000");
    ADD_TEST(11*60*60*1000, "11:00:00,000");

    /* Complicated number */
    ADD_TEST(5
             + 500
             + 5000
             + 50*60*1000
             + 10*60*60*1000,  "10:50:05,505");

    /* Very large time periods */
    ADD_TEST(5
             + 500
             + 5000
             + 50*60*1000
             + 111*60*60*1000,  "111:50:05,505");

    /* Negative numbers */
    ADD_TEST(-5,             "-00:00:00,005");
    ADD_TEST(-500,           "-00:00:00,500");
    ADD_TEST(-5000,          "-00:00:05,000");
    ADD_TEST(-50*60*1000,    "-00:50:00,000");
    ADD_TEST(-60*60*1000,    "-01:00:00,000");

    /* Negative complicated numbers */
    ADD_TEST(-5
             - 500
             - 5000
             - 50*60*1000
             - 10*60*60*1000,  "-10:50:05,505");

    /* Very large time periods */
    ADD_TEST(-5
             - 500
             - 5000
             - 50*60*1000
             - 111*60*60*1000,  "-111:50:05,505");
}

void TestFormatRelativeTime::formatTime()
{
    QFETCH(qint64, ticks);
    QFETCH(QString, result);

    QString str = FormatRelativeTime::formatTime(ticks);

    QCOMPARE(str, result);
}

void TestFormatRelativeTime::formatTime_Absolute_data()
{
    QTest::addColumn<qint64>("ticks");
    QTest::addColumn<QString>("result");

    /* Workaround to get around different time zones */
    if (QDateTime().timeZone().country() == QLocale::Belgium)
    {
        ADD_TEST(1602935825011,      "17/10/2020\n13:57:05,011");
        ADD_TEST(1602899825011,      "17/10/2020\n03:57:05,011");
    }
    else
    {
        ADD_TEST(1602935825011,      "17/10/2020\n11:57:05,011");
        ADD_TEST(1602899825011,      "17/10/2020\n01:57:05,011");
    }
}

void TestFormatRelativeTime::formatTime_Absolute()
{
    QFETCH(qint64, ticks);
    QFETCH(QString, result);

    QString str = FormatRelativeTime::formatTime(ticks);

    QCOMPARE(str, result);
}

QTEST_GUILESS_MAIN(TestFormatRelativeTime)
