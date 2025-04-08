
#include "tst_util.h"

#include "util/util.h"

#include <QtTest/QtTest>

#define ADD_TEST(value, result)      QTest::newRow(QLocale().toString(value).toLocal8Bit()) << static_cast<double>(value) << result

void TestUtil::init()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

void TestUtil::cleanup()
{

}

void TestUtil::formatDouble_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<QString>("result");

    ADD_TEST(3,             "3");
    ADD_TEST(4.00,          "4");
    ADD_TEST(3.500,         "3,5");
    ADD_TEST(31234,         "31234");

    ADD_TEST(11123456789,   "11123456789");
    ADD_TEST(3.123456789,   "3,123456789");

    ADD_TEST(0.0001,        "0,0001");
    ADD_TEST(0.00001,       "1E-05");
    ADD_TEST(0.000001,      "1E-06");

    ADD_TEST(0.00021,       "0,00021");
    ADD_TEST(0.000099,      "9,9E-05");

    ADD_TEST(0xFFFFFFFF,   "4294967295");
}

void TestUtil::formatDouble()
{
    QFETCH(double, value);
    QFETCH(QString, result);

    QString act = Util::formatDoubleForExport(value);

    QCOMPARE(act, result);
}

QTEST_GUILESS_MAIN(TestUtil)
