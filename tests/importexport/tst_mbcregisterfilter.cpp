#include "tst_mbcregisterfilter.h"

#include "models/mbcregisterfilter.h"
#include "models/mbcregistermodel.h"

#include <QTest>

void TestMbcRegisterFilter::init()
{
    _pMbcRegisterModel = new MbcRegisterModel();

    _pFilterProxy = new MbcRegisterFilter();
    _pFilterProxy->setSourceModel(_pMbcRegisterModel);

    QList<MbcRegisterData> mbcRegisterList;
    QStringList tabList = QStringList() << QStringLiteral("tab1") << QStringLiteral("tab2");

    mbcRegisterList.append(MbcRegisterData(40001, ModbusDataType::Type::UNSIGNED_16, "Test1", 0, true, 0));
    mbcRegisterList.append(MbcRegisterData(40002, ModbusDataType::Type::UNSIGNED_16, "Test2", 0, true, 0));
    mbcRegisterList.append(MbcRegisterData(41002, ModbusDataType::Type::UNSIGNED_16, "Test3", 1, true, 0));
    mbcRegisterList.append(MbcRegisterData(41003, ModbusDataType::Type::UNSIGNED_16, "Test4", 1, true, 0));

    _pMbcRegisterModel->fill(mbcRegisterList, tabList);
}

void TestMbcRegisterFilter::cleanup()
{
    delete _pFilterProxy;
    delete _pMbcRegisterModel;
}

void TestMbcRegisterFilter::noFilter()
{
    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()));
}

void TestMbcRegisterFilter::tabFilter()
{
    _pFilterProxy->setTab("tab1");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()) == false);
}

void TestMbcRegisterFilter::textFilter()
{
    _pFilterProxy->setTextFilter("Test4");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()));
}

void TestMbcRegisterFilter::textAddressFilter()
{
    _pFilterProxy->setTextFilter("4100");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()));
}

void TestMbcRegisterFilter::tabTextFilter()
{
    _pFilterProxy->setTextFilter("2");
    _pFilterProxy->setTab("tab1");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()) == false);
}

void TestMbcRegisterFilter::tabNoFilterShowsAll()
{
    // Explicitly setting the "No Filter" value should show all rows regardless of tab
    _pFilterProxy->setTab(MbcRegisterFilter::cTabNoFilter);

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()));
}

void TestMbcRegisterFilter::tabFilterHidesOtherTabs()
{
    _pFilterProxy->setTab("tab2");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()));
}

void TestMbcRegisterFilter::textFilterCaseInsensitive()
{
    // Filter is case-insensitive — "test1" should match "Test1"
    _pFilterProxy->setTextFilter("test1");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()) == false);
}

void TestMbcRegisterFilter::addressFilterPartialMatch()
{
    // Partial address match — "400" should match 40001 and 40002 but not 41002/41003
    _pFilterProxy->setTextFilter("400");

    QVERIFY(_pFilterProxy->filterAcceptsRow(0, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(1, QModelIndex()));
    QVERIFY(_pFilterProxy->filterAcceptsRow(2, QModelIndex()) == false);
    QVERIFY(_pFilterProxy->filterAcceptsRow(3, QModelIndex()) == false);
}

void TestMbcRegisterFilter::emptyModelAcceptsNothing()
{
    // An empty model with no rows should accept no row indices
    MbcRegisterModel emptyModel;
    MbcRegisterFilter emptyFilter;
    emptyFilter.setSourceModel(&emptyModel);

    QVERIFY(emptyFilter.filterAcceptsRow(0, QModelIndex()) == false);
}

void TestMbcRegisterFilter::outOfRangeRowRejected()
{
    // Row index beyond model size should be rejected
    QVERIFY(_pFilterProxy->filterAcceptsRow(100, QModelIndex()) == false);
}

QTEST_GUILESS_MAIN(TestMbcRegisterFilter)
