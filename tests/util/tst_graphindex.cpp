
#include "tst_graphindex.h"

#include "util/graphindex.h"

#include <QMetaType>
#include <QTest>

void TestGraphIndex::initTestCase()
{
    qRegisterMetaType<GraphIdx>("GraphIdx");
    qRegisterMetaType<ActiveIdx>("ActiveIdx");
}

// ---- GraphIdx ----

void TestGraphIndex::graphIdx_defaultConstructed_isInvalid()
{
    GraphIdx idx;
    QVERIFY(!idx.isValid());
}

void TestGraphIndex::graphIdx_explicitValue_isValid()
{
    GraphIdx idx(0);
    QVERIFY(idx.isValid());

    GraphIdx idx2(42);
    QVERIFY(idx2.isValid());
}

void TestGraphIndex::graphIdx_explicitValue_storesValue()
{
    GraphIdx idx(7);
    QCOMPARE(idx.v, 7);
}

void TestGraphIndex::graphIdx_noneConstant_matchesDefault()
{
    QCOMPARE(GraphIdx::NONE, -1);
    GraphIdx idx;
    QCOMPARE(idx.v, GraphIdx::NONE);
}

void TestGraphIndex::graphIdx_equality_sameValue()
{
    QVERIFY(GraphIdx(3) == GraphIdx(3));
}

void TestGraphIndex::graphIdx_equality_differentValues()
{
    QVERIFY(!(GraphIdx(2) == GraphIdx(5)));
}

void TestGraphIndex::graphIdx_inequality()
{
    QVERIFY(GraphIdx(1) != GraphIdx(2));
    QVERIFY(!(GraphIdx(1) != GraphIdx(1)));
}

void TestGraphIndex::graphIdx_lessThan()
{
    QVERIFY(GraphIdx(0) < GraphIdx(1));
    QVERIFY(!(GraphIdx(1) < GraphIdx(0)));
    QVERIFY(!(GraphIdx(1) < GraphIdx(1)));
}

// ---- ActiveIdx ----

void TestGraphIndex::activeIdx_defaultConstructed_isInvalid()
{
    ActiveIdx idx;
    QVERIFY(!idx.isValid());
}

void TestGraphIndex::activeIdx_explicitValue_isValid()
{
    ActiveIdx idx(0);
    QVERIFY(idx.isValid());

    ActiveIdx idx2(5);
    QVERIFY(idx2.isValid());
}

void TestGraphIndex::activeIdx_explicitValue_storesValue()
{
    ActiveIdx idx(4);
    QCOMPARE(idx.v, 4);
}

void TestGraphIndex::activeIdx_noneConstant_matchesDefault()
{
    QCOMPARE(ActiveIdx::NONE, -1);
    ActiveIdx idx;
    QCOMPARE(idx.v, ActiveIdx::NONE);
}

void TestGraphIndex::activeIdx_equality_sameValue()
{
    QVERIFY(ActiveIdx(2) == ActiveIdx(2));
}

void TestGraphIndex::activeIdx_equality_differentValues()
{
    QVERIFY(!(ActiveIdx(0) == ActiveIdx(3)));
}

void TestGraphIndex::activeIdx_inequality()
{
    QVERIFY(ActiveIdx(0) != ActiveIdx(1));
    QVERIFY(!(ActiveIdx(0) != ActiveIdx(0)));
}

void TestGraphIndex::activeIdx_lessThan()
{
    QVERIFY(ActiveIdx(0) < ActiveIdx(1));
    QVERIFY(!(ActiveIdx(1) < ActiveIdx(0)));
    QVERIFY(!(ActiveIdx(1) < ActiveIdx(1)));
}

// ---- Meta-type registration ----

void TestGraphIndex::metatype_graphIdx_isRegistered()
{
    QVERIFY(QMetaType::fromType<GraphIdx>().isValid());
}

void TestGraphIndex::metatype_activeIdx_isRegistered()
{
    QVERIFY(QMetaType::fromType<ActiveIdx>().isValid());
}

/*
 * Type-safety invariant (compile-time, not runtime):
 * GraphIdx and ActiveIdx are distinct types. Passing a GraphIdx where an
 * ActiveIdx is expected — or vice versa — is a compile error. This is
 * enforced by the compiler and cannot be expressed as a runtime assertion.
 */

QTEST_GUILESS_MAIN(TestGraphIndex)
