
#include <QtTest/QtTest>
#include <QSignalSpy>

#include "gmock/gmock.h"
#include <gmock/gmock-matchers.h>

#include "../mocks/gmockutils.h"

#include "testmbcregistermodel.h"

#include "mbcregistermodel.h"

using ::testing::Return;
using namespace testing;

static const quint32 cColumnSelected = 0;
static const quint32 cColumnAddress = 1;
static const quint32 cColumnText = 2;
static const quint32 cColumnUnsigned = 3;
static const quint32 cColumnTab = 4;
static const quint32 cColumnDecimals = 5;
static const quint32 cColumnCnt = 6;

void TestMbcRegisterModel::init()
{
    // IMPORTANT: This must be called before any mock object constructors
    GMockUtils::InitGoogleMock();
}

void TestMbcRegisterModel::cleanup()
{

}

void TestMbcRegisterModel::rowCount()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QCOMPARE(pMbcRegisterModel->rowCount(), 0);

    fillModel(pMbcRegisterModel);

    QVERIFY(pMbcRegisterModel->rowCount() != 0);

    /* tested thoroughly in fill test case */
}


void TestMbcRegisterModel::columnCount()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QCOMPARE(pMbcRegisterModel->columnCount(QModelIndex()), cColumnCnt);
}

void TestMbcRegisterModel::headerData()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QCOMPARE(pMbcRegisterModel->headerData(cColumnSelected, Qt::Horizontal, Qt::DisplayRole).toString(), QString(""));
    QCOMPARE(pMbcRegisterModel->headerData(cColumnAddress, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Address"));
    QCOMPARE(pMbcRegisterModel->headerData(cColumnText, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Text"));
    QCOMPARE(pMbcRegisterModel->headerData(cColumnUnsigned, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Unsigned"));
    QCOMPARE(pMbcRegisterModel->headerData(cColumnTab, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Tab"));
    QCOMPARE(pMbcRegisterModel->headerData(cColumnDecimals, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Decimals"));

    QCOMPARE(pMbcRegisterModel->headerData(cColumnCnt, Qt::Horizontal, Qt::DisplayRole), QVariant());
}

void TestMbcRegisterModel::flagsEnabled()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    fillModel(pMbcRegisterModel);

    QModelIndex modelIdx = pMbcRegisterModel->index(0,0);
    Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;

    QCOMPARE(pMbcRegisterModel->flags(QModelIndex()), Qt::NoItemFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnSelected)), Qt::ItemIsUserCheckable | enabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnUnsigned)), Qt::ItemIsUserCheckable | enabledFlags);

    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnText)), enabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnTab)), enabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnDecimals)), enabledFlags);
}


void TestMbcRegisterModel::flagsDisabled()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, false, 0) /* Not readable */
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QModelIndex modelIdx = pMbcRegisterModel->index(0,0);
    Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    QCOMPARE(pMbcRegisterModel->flags(QModelIndex()), Qt::NoItemFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnSelected)), Qt::ItemIsUserCheckable | disabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnUnsigned)), Qt::ItemIsUserCheckable | disabledFlags);

    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), disabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnText)), disabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnTab)), disabledFlags);
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnDecimals)), disabledFlags);

}

void TestMbcRegisterModel::setData()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();
    fillModel(pMbcRegisterModel);

    QSignalSpy spy(pMbcRegisterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    QModelIndex modelIdx = pMbcRegisterModel->index(0, cColumnSelected);

    /* Check failures */
    QCOMPARE(pMbcRegisterModel->setData(QModelIndex(), QVariant(), Qt::CheckStateRole), false);
    QCOMPARE(spy.count(), 0);

    QCOMPARE(pMbcRegisterModel->setData(modelIdx.sibling(0, cColumnUnsigned), QVariant(), Qt::CheckStateRole), false);
    QCOMPARE(spy.count(), 0);

    // At least 2 rows required for this test
    QVERIFY(pMbcRegisterModel->rowCount() >= 2);

    QModelIndex modelIdxFirstRow = pMbcRegisterModel->index(0, cColumnSelected);
    QModelIndex modelIdxSecondRow = pMbcRegisterModel->index(1, cColumnSelected);

    QCOMPARE(pMbcRegisterModel->data(modelIdxFirstRow, Qt::CheckStateRole), Qt::Unchecked);
    QCOMPARE(pMbcRegisterModel->data(modelIdxSecondRow, Qt::CheckStateRole), Qt::Unchecked);

    QCOMPARE(pMbcRegisterModel->setData(modelIdxFirstRow, QVariant(Qt::Checked), Qt::CheckStateRole), true);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(0)).row(), 0); /* First argument (start index) */
    QCOMPARE(qvariant_cast<QModelIndex>(arguments.at(1)).row(), 1); /* Second argument (end index) */

    QCOMPARE(pMbcRegisterModel->data(modelIdxFirstRow, Qt::CheckStateRole), Qt::Checked);
    QCOMPARE(pMbcRegisterModel->data(modelIdxSecondRow, Qt::CheckStateRole), Qt::Unchecked);
}

void TestMbcRegisterModel::disableAlreadyStagedRegisterAddress()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40001, true, "Test1_2", 2, false, true, 0)
            << MbcRegisterData(40002, false, "Test2", 1, true, true, 0)
            << MbcRegisterData(40011, false, "Test11", 2, false, false, 0); // Disabled: not readable
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QModelIndex modelIdx = pMbcRegisterModel->index(0, 0);
    const Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
    const Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    // *** Start all unchecked
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 0);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Check first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    // Check disabled second
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "Already selected address");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), disabledFlags);


    // *** Uncheck first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    // Check all enabled
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 0);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Check second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    // Check disabled first
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "Already selected address");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), disabledFlags);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Uncheck second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    // Check all enabled
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 0);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);

}

void TestMbcRegisterModel::fillData()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();
    QSignalSpy resetSignalSpy(pMbcRegisterModel, SIGNAL(modelReset()));

    /*-- Test fill and expected signals --*/

    QCOMPARE(pMbcRegisterModel->rowCount(), 0);

    fillModel(pMbcRegisterModel);

    QVERIFY(pMbcRegisterModel->rowCount() != 0);
    QCOMPARE(resetSignalSpy.count(), 0);

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, false, "Test2", 1, true, true, 0)
            << MbcRegisterData(40004, true, "Test4", 1, false, true, 0)
            << MbcRegisterData(40005, false, "Test5", 0, false, true, 2)
            << MbcRegisterData(40010, true, "Test10", 2, false, true, 3)
            << MbcRegisterData(40011, false, "Test11", 2, false, false, 0) // Disabled: not readable
            << MbcRegisterData(40002, false, "Test6", 0, false, true, 0)
            << MbcRegisterData(40004, false, "Test13", 0, false, true, 0);

    QStringList tabList = QStringList() << QString("Tab0") << QString("Tab1") << QString("Tab2");
    QSignalSpy rowsInsertedSpy(pMbcRegisterModel, SIGNAL(rowsInserted(const QModelIndex, int, int)));

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QCOMPARE(pMbcRegisterModel->rowCount(), mbcRegisterList.size());
    QCOMPARE(resetSignalSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);

    QList<QVariant> arguments = rowsInsertedSpy.takeFirst(); // take the first signal
    QCOMPARE(arguments.at(1), 0);
    QCOMPARE(arguments.at(2), mbcRegisterList.size() - 1);

    /*-- Test result of fill with data function  --*/

    QModelIndex modelIdx = pMbcRegisterModel->index(0, 0);

    const Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
    const Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    int row = 0;

    row = 0;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 1;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 2;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 3;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 4;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 5;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "Not readable");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), disabledFlags); // flags

    row = 6;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 7;
    QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    QCOMPARE(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags


    /* Loop when possible */
    for (int rowIdx = 0; rowIdx < mbcRegisterList.size(); rowIdx++)
    {
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnSelected), Qt::CheckStateRole), Qt::Unchecked);
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnAddress), Qt::DisplayRole), mbcRegisterList[rowIdx].registerAddress());
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnText), Qt::DisplayRole), mbcRegisterList[rowIdx].name());
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnTab), Qt::DisplayRole), tabList[mbcRegisterList[rowIdx].tabIdx()]);
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnDecimals), Qt::DisplayRole), mbcRegisterList[rowIdx].decimals());

        Qt::CheckState state = mbcRegisterList[rowIdx].isUnsigned() ? Qt::Checked : Qt::Unchecked;
        QCOMPARE(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnUnsigned), Qt::CheckStateRole), state);
    }
}

void TestMbcRegisterModel::reset()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();
    QSignalSpy resetSignalSpy(pMbcRegisterModel, SIGNAL(modelReset()));

    fillModel(pMbcRegisterModel);
    QVERIFY(pMbcRegisterModel->rowCount() != 0);

    pMbcRegisterModel->reset();

    QCOMPARE(pMbcRegisterModel->rowCount(), 0);
    QCOMPARE(resetSignalSpy.count(), 1);
}

void TestMbcRegisterModel::selectedRegisterListAndCount()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    // At least 2 rows required for this test
    QVERIFY(pMbcRegisterModel->rowCount() >= 2);

    QList<GraphData> graphListRef;
    GraphData graphData;
    graphData.setActive(true);
    graphData.setRegisterAddress(40001);
    graphData.setLabel("Test1");
    graphData.setUnsigned(true);
    graphData.setBit32(false);
    graphData.setExpression(QStringLiteral("VAL"));
    graphListRef.append(graphData);

    graphData.setActive(true);
    graphData.setRegisterAddress(40002);
    graphData.setLabel("Test2");
    graphData.setUnsigned(true);
    graphData.setBit32(false);
    graphData.setExpression(QStringLiteral("VAL"));
    graphListRef.append(graphData);


    QList<GraphData> graphList;
    QModelIndex modelIdx;

    // Start all unchecked
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 0);

    // Check first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 1);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[0].isActive(), graphListRef[0].isActive());
    QCOMPARE(graphList[0].registerAddress(), graphListRef[0].registerAddress());
    QCOMPARE(graphList[0].label(), graphListRef[0].label());
    QCOMPARE(graphList[0].isUnsigned(), graphListRef[0].isUnsigned());
    QCOMPARE(graphList[0].isBit32(), graphListRef[0].isBit32());
    QCOMPARE(graphList[0].expression(), graphListRef[0].expression());

    // Check second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 2);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[0].isActive(), graphListRef[0].isActive());
    QCOMPARE(graphList[0].registerAddress(), graphListRef[0].registerAddress());
    QCOMPARE(graphList[0].label(), graphListRef[0].label());
    QCOMPARE(graphList[0].isUnsigned(), graphListRef[0].isUnsigned());
    QCOMPARE(graphList[0].isBit32(), graphListRef[0].isBit32());
    QCOMPARE(graphList[0].expression(), graphListRef[0].expression());

    QCOMPARE(graphList[1].isActive(), graphListRef[1].isActive());
    QCOMPARE(graphList[1].registerAddress(), graphListRef[1].registerAddress());
    QCOMPARE(graphList[1].label(), graphListRef[1].label());
    QCOMPARE(graphList[1].isUnsigned(), graphListRef[1].isUnsigned());
    QCOMPARE(graphList[1].isBit32(), graphListRef[1].isBit32());
    QCOMPARE(graphList[1].expression(), graphListRef[1].expression());

    // Uncheck first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 1);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[0].isActive(), graphListRef[1].isActive());
    QCOMPARE(graphList[0].registerAddress(), graphListRef[1].registerAddress());
    QCOMPARE(graphList[0].label(), graphListRef[1].label());
    QCOMPARE(graphList[0].isUnsigned(), graphListRef[1].isUnsigned());
    QCOMPARE(graphList[0].isBit32(), graphListRef[1].isBit32());
    QCOMPARE(graphList[0].expression(), graphListRef[0].expression());


    // Uncheck second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 0);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());
}

void TestMbcRegisterModel::selectedRegisterListAndCount32()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, true, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QVERIFY(pMbcRegisterModel->rowCount() >= 1);

    QList<GraphData> graphList;
    QModelIndex modelIdx;

    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 1);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[0].isActive(), true);
    QCOMPARE(graphList[0].registerAddress(), 40001);
    QCOMPARE(graphList[0].label(), "Test1");
    QCOMPARE(graphList[0].isUnsigned(), true);
    QCOMPARE(graphList[0].isBit32(), true);
    QCOMPARE(graphList[0].expression(), "VAL");
}

void TestMbcRegisterModel::selectedRegisterListDecimals()
{
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel();

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 1)
            << MbcRegisterData(40003, true, "Test3", 0, false, true, 2);
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QVERIFY(pMbcRegisterModel->rowCount() >= 3);

    QList<GraphData> graphList;
    QModelIndex modelIdx;

    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    modelIdx = pMbcRegisterModel->index(2, cColumnSelected);
    QCOMPARE(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), 3);
    QCOMPARE(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[0].isActive(), true);
    QCOMPARE(graphList[0].registerAddress(), 40001);
    QCOMPARE(graphList[0].expression(), "VAL");

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[1].isActive(), true);
    QCOMPARE(graphList[1].registerAddress(), 40002);
    QCOMPARE(graphList[1].expression(), "VAL/10");

    graphList = pMbcRegisterModel->selectedRegisterList();
    QCOMPARE(graphList[2].isActive(), true);
    QCOMPARE(graphList[2].registerAddress(), 40003);
    QCOMPARE(graphList[2].expression(), "VAL/100");
}

void TestMbcRegisterModel::fillModel(MbcRegisterModel * pMbcRegisterModel)
{
    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    pMbcRegisterModel->fill(mbcRegisterList, tabList);
}

QTEST_GUILESS_MAIN(TestMbcRegisterModel)
