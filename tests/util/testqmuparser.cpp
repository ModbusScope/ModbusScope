
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

    /* Decision logic */
    ADD_TEST("0 || 2",          1    );
    ADD_TEST("0 || 0",          0    );
    ADD_TEST("1 && 0",          0    );
    ADD_TEST("1 && 123",        1    );
    ADD_TEST("2 < 3",           1    );
    ADD_TEST("3 > 2",           1    );
    ADD_TEST("0 <= 1",          1    );
    ADD_TEST("3 >= 2",          1    );
    ADD_TEST("1 != 0",          1    );
    ADD_TEST("1 == 1",          1    );
    ADD_TEST("if(5%2,1,0)",     1    );
    ADD_TEST("if(4%2,1,0)",     0    );

}

void TestQMuParser::evaluate()
{
    QFETCH(QString, expression);
    QFETCH(double, result);

    QMuParser parser(expression);

    parser.evaluate();

    QCOMPARE(parser.result(), result);
}

QTEST_GUILESS_MAIN(TestQMuParser)
