
#include "tst_mbcregisterfilter.h"

#include "models/mbcregisterfilter.h"
#include "models/mbcregistermodel.h"

#include <QtTest/QtTest>

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

QTEST_GUILESS_MAIN(TestMbcRegisterFilter)
