
#include <QtTest/QtTest>

#include "qmuparser.h"

#include "testqmuparser.h"

#define ADD_TEST(expr, result)      QTest::newRow(expr) << QString(expr) << static_cast<double>(result)

void TestQMuParser::init()
{

}

void TestQMuParser::cleanup()
{

}

void TestQMuParser::evaluate_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<double>("result");

    /* binary */
    ADD_TEST("1 | 2",           3    );
    ADD_TEST("123 & 456",       72   );
    ADD_TEST("1 << 3",          8    );
    ADD_TEST("8 >> 3",          1    );

    /* Arithmetic */
    ADD_TEST("16 / 4",          4    );
    ADD_TEST("9 * 4",           36   );
    ADD_TEST("9 % 4",           1    );
    ADD_TEST("-10+1",           -9   );
    ADD_TEST("1+2*3",           7    );
    ADD_TEST("(1+2)*3",         9    );
    ADD_TEST("3+-3^2",          -6   );
    ADD_TEST("2^2^3",           256  );
    ADD_TEST("-2^2",            -4   );
    ADD_TEST("(-2)^2",          4    );

    /* Hexadecimal */
    ADD_TEST("0xff",            255  );
    ADD_TEST("0xffFF",          65535  );
    ADD_TEST("0xFFFFffff",      4294967295  );
    ADD_TEST("0xff",            255  );
    ADD_TEST("10+0xff",         265  );
    ADD_TEST("0xff+10",         265  );
    ADD_TEST("10*0xff",         2550 );
    ADD_TEST("0xff*10",         2550 );
    ADD_TEST("10+0xff+1",       266  );
    ADD_TEST("1+0xff+10",       266  );
    ADD_TEST("0x12345678",      305419896  );
    ADD_TEST("0xABCDEF",        11259375  );

    /* Floating point */
    ADD_TEST("15 / 4",          3.75   );
    ADD_TEST("1 / 1000",        0.001  );
    ADD_TEST("15.6 + 5.5",      21.1   );
    ADD_TEST("15.6 - 5.5",      10.1   );
    ADD_TEST("3.33 * 3",        9.99   );

    /* Floating point with binary/integer operations: fraction part is removed and ignored */
    ADD_TEST("15.5 | 0xF",     15  );
    ADD_TEST("16.5 & 0x10",     16  );
    ADD_TEST("1.3 << 3",        8    );
    ADD_TEST("8.6 >> 3",        1    );

    ADD_TEST("9.5 % 4",         1    );

}

void TestQMuParser::evaluate()
{
    QFETCH(QString, expression);
    QFETCH(double, result);

    QMuParser parser(expression);

    bool bSuccess = parser.evaluate();

    QCOMPARE(parser.result(), result);
    QVERIFY(parser.isSuccess());
    QVERIFY(bSuccess);
}

QTEST_GUILESS_MAIN(TestQMuParser)
