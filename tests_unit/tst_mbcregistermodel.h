#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <gmock/gmock-matchers.h>

#include "mockgraphdatamodel.h"
#include <QSignalSpy>

#include "../src/models/mbcregistermodel.h"


using ::testing::Return;
using namespace testing;

static const quint32 cColumnSelected = 0;
static const quint32 cColumnAddress = 1;
static const quint32 cColumnText = 2;
static const quint32 cColumnUnsigned = 3;
static const quint32 cColumnTab = 4;
static const quint32 cColumnDecimals = 5;
static const quint32 cColumnCnt = 6;

void fillModel(MockGraphDataModel * pGraphDataModel, MbcRegisterModel * pMbcRegisterModel, bool bAlreadyPresent)
{
    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    EXPECT_CALL(*pGraphDataModel, isPresent(_, _))
        .WillRepeatedly(Return(bAlreadyPresent));

    pMbcRegisterModel->fill(mbcRegisterList, tabList);
}

TEST(MbcRegisterModel, rowCount)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    EXPECT_EQ(pMbcRegisterModel->rowCount(), 0);

    fillModel(&graphDataModel, pMbcRegisterModel, false);

    EXPECT_NE(pMbcRegisterModel->rowCount(), 0);

    /* tested thoroughly in fill test case */
}


TEST(MbcRegisterModel, columnCount)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    EXPECT_EQ(pMbcRegisterModel->columnCount(QModelIndex()), cColumnCnt);
}

TEST(MbcRegisterModel, headerData)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnSelected, Qt::Horizontal, Qt::DisplayRole).toString(), QString(""));
    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnAddress, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Address"));
    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnText, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Text"));
    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnUnsigned, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Unsigned"));
    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnTab, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Tab"));
    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnDecimals, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Decimals"));

    EXPECT_EQ(pMbcRegisterModel->headerData(cColumnCnt, Qt::Horizontal, Qt::DisplayRole), QVariant());
}

TEST(MbcRegisterModel, flagsEnabled)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    fillModel(&graphDataModel, pMbcRegisterModel, false);

    QModelIndex modelIdx = pMbcRegisterModel->index(0,0);
    Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;

    EXPECT_EQ(pMbcRegisterModel->flags(QModelIndex()), Qt::NoItemFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnSelected)), Qt::ItemIsUserCheckable | enabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnUnsigned)), Qt::ItemIsUserCheckable | enabledFlags);

    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnText)), enabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnTab)), enabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnDecimals)), enabledFlags);
}


TEST(MbcRegisterModel, flagsDisabled)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);
    fillModel(&graphDataModel, pMbcRegisterModel, true);


    QModelIndex modelIdx = pMbcRegisterModel->index(0,0);
    Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    EXPECT_EQ(pMbcRegisterModel->flags(QModelIndex()), Qt::NoItemFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnSelected)), Qt::ItemIsUserCheckable | disabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnUnsigned)), Qt::ItemIsUserCheckable | disabledFlags);

    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), disabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnText)), disabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnTab)), disabledFlags);
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnDecimals)), disabledFlags);

}

TEST(MbcRegisterModel, setData)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);
    fillModel(&graphDataModel, pMbcRegisterModel, false);

    QSignalSpy spy(pMbcRegisterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    QModelIndex modelIdx = pMbcRegisterModel->index(0, cColumnSelected);

    /* Check failures */
    EXPECT_EQ(pMbcRegisterModel->setData(QModelIndex(), QVariant(), Qt::CheckStateRole), false);
    EXPECT_EQ(spy.count(), 0);

    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx.sibling(0, cColumnUnsigned), QVariant(), Qt::CheckStateRole), false);
    EXPECT_EQ(spy.count(), 0);

    // At least 2 rows required for this test
    EXPECT_GE(pMbcRegisterModel->rowCount(), 2);

    QModelIndex modelIdxFirstRow = pMbcRegisterModel->index(0, cColumnSelected);
    QModelIndex modelIdxSecondRow = pMbcRegisterModel->index(1, cColumnSelected);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdxFirstRow, Qt::CheckStateRole), Qt::Unchecked);
    EXPECT_EQ(pMbcRegisterModel->data(modelIdxSecondRow, Qt::CheckStateRole), Qt::Unchecked);

    EXPECT_EQ(pMbcRegisterModel->setData(modelIdxFirstRow, QVariant(Qt::Checked), Qt::CheckStateRole), true);
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    EXPECT_EQ(qvariant_cast<QModelIndex>(arguments.at(0)).row(), 0); /* First argument (start index) */
    EXPECT_EQ(qvariant_cast<QModelIndex>(arguments.at(1)).row(), 1); /* Second argument (end index) */

    EXPECT_EQ(pMbcRegisterModel->data(modelIdxFirstRow, Qt::CheckStateRole), Qt::Checked);
    EXPECT_EQ(pMbcRegisterModel->data(modelIdxSecondRow, Qt::CheckStateRole), Qt::Unchecked);
}

TEST(MbcRegisterModel, disableAlreadyStagedRegisterAddress)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40001, true, "Test1_2", 2, false, true, 0)
            << MbcRegisterData(40002, false, "Test2", 1, true, true, 0) // Disabled: 32 bit
            << MbcRegisterData(40011, false, "Test11", 2, false, false, 0); // Disabled: not readable
    QStringList tabList = QStringList() << QString("Tab0");

    EXPECT_CALL(graphDataModel, isPresent(_, _))
        .WillRepeatedly(Return(false));

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    QModelIndex modelIdx = pMbcRegisterModel->index(0, 0);
    const Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
    const Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    // *** Start all unchecked
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 0);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Check first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    // Check disabled second
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "Already selected address");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), disabledFlags);


    // *** Uncheck first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    // Check all enabled
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 0);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Check second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    // Check disabled first
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "Already selected address");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), disabledFlags);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);


    // *** Uncheck second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    // Check all enabled
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 0);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(0, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(0, cColumnAddress)), enabledFlags);

    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(1, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(1, cColumnAddress)), enabledFlags);

}

TEST(MbcRegisterModel, fillData)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);
    QSignalSpy resetSignalSpy(pMbcRegisterModel, SIGNAL(modelReset()));

    /*-- Test fill and expected signals --*/

    EXPECT_EQ(pMbcRegisterModel->rowCount(), 0);

    fillModel(&graphDataModel, pMbcRegisterModel, false);

    EXPECT_NE(pMbcRegisterModel->rowCount(), 0);
    EXPECT_EQ(resetSignalSpy.count(), 0);

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0) // Disabled: Will be already present
            << MbcRegisterData(40002, false, "Test2", 1, true, true, 0) // Disabled: 32 bit
            << MbcRegisterData(40004, true, "Test4", 1, false, true, 0)
            << MbcRegisterData(40005, false, "Test5", 0, false, true, 2)
            << MbcRegisterData(40010, true, "Test10", 2, false, true, 3)
            << MbcRegisterData(40011, false, "Test11", 2, false, false, 0) // Disabled: not readable
            << MbcRegisterData(40002, false, "Test6", 0, false, true, 0) // Enabled (Duplicate, but other is 32 bit so ignored)
            << MbcRegisterData(40004, false, "Test13", 0, false, true, 0); // Enabled: Is duplicate, but will only be disabled when same reg address is selected

    QStringList tabList = QStringList() << QString("Tab0") << QString("Tab1") << QString("Tab2");
    QSignalSpy rowsInsertedSpy(pMbcRegisterModel, SIGNAL(rowsInserted(const QModelIndex, int, int)));

    EXPECT_CALL(graphDataModel, isPresent(_, _))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    pMbcRegisterModel->fill(mbcRegisterList, tabList);

    EXPECT_EQ(pMbcRegisterModel->rowCount(), mbcRegisterList.size());
    EXPECT_EQ(resetSignalSpy.count(), 1);
    EXPECT_EQ(rowsInsertedSpy.count(), 1);

    QList<QVariant> arguments = rowsInsertedSpy.takeFirst(); // take the first signal
    EXPECT_EQ(arguments.at(1), 0);
    EXPECT_EQ(arguments.at(2), mbcRegisterList.size() - 1);

    /*-- Test result of fill with data function  --*/

    QModelIndex modelIdx = pMbcRegisterModel->index(0, 0);

    const Qt::ItemFlags enabledFlags = Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
    const Qt::ItemFlags disabledFlags = Qt::NoItemFlags;

    int row = 0;

    row = 0;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "Already added address");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), disabledFlags); // flags

    row = 1;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "32 bit register is not supported");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), disabledFlags); // flags

    row = 2;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 3;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 4;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 5;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "Not readable");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), disabledFlags); // flags

    row = 6;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags

    row = 7;
    EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(row, cColumnSelected), Qt::ToolTipRole).toString(), "");
    EXPECT_EQ(pMbcRegisterModel->flags(modelIdx.sibling(row, cColumnAddress)), enabledFlags); // flags


    /* Loop when possible */
    for (int rowIdx = 0; rowIdx < mbcRegisterList.size(); rowIdx++)
    {
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnSelected), Qt::CheckStateRole), Qt::Unchecked);
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnAddress), Qt::DisplayRole), mbcRegisterList[rowIdx].registerAddress());
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnText), Qt::DisplayRole), mbcRegisterList[rowIdx].name());
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnTab), Qt::DisplayRole), tabList[mbcRegisterList[rowIdx].tabIdx()]);
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnDecimals), Qt::DisplayRole), mbcRegisterList[rowIdx].decimals());

        Qt::CheckState state = mbcRegisterList[rowIdx].isUnsigned() ? Qt::Checked : Qt::Unchecked;
        EXPECT_EQ(pMbcRegisterModel->data(modelIdx.sibling(rowIdx, cColumnUnsigned), Qt::CheckStateRole), state);
    }
}

TEST(MbcRegisterModel, reset)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);
    QSignalSpy resetSignalSpy(pMbcRegisterModel, SIGNAL(modelReset()));

    fillModel(&graphDataModel, pMbcRegisterModel, false);
    EXPECT_NE(pMbcRegisterModel->rowCount(), 0);

    pMbcRegisterModel->reset();

    EXPECT_EQ(pMbcRegisterModel->rowCount(), 0);
    EXPECT_EQ(resetSignalSpy.count(), 1);
}

TEST(MbcRegisterModel, selectedRegisterListAndCount)
{
    MockGraphDataModel graphDataModel;
    MbcRegisterModel * pMbcRegisterModel = new MbcRegisterModel(&graphDataModel);

    QList<MbcRegisterData> mbcRegisterList = QList<MbcRegisterData>()
            << MbcRegisterData(40001, true, "Test1", 0, false, true, 0)
            << MbcRegisterData(40002, true, "Test2", 0, false, true, 0);
    QStringList tabList = QStringList() << QString("Tab0");

    EXPECT_CALL(graphDataModel, isPresent(_, _))
        .WillRepeatedly(Return(false));

    pMbcRegisterModel->fill(mbcRegisterList, tabList);


    // At least 2 rows required for this test
    EXPECT_GE(pMbcRegisterModel->rowCount(), 2);

    QList<GraphData> graphListRef;
    GraphData graphData;
    graphData.setActive(true);
    graphData.setRegisterAddress(40001);
    graphData.setLabel("Test1");
    graphData.setUnsigned(true);
    graphListRef.append(graphData);

    graphData.setActive(true);
    graphData.setRegisterAddress(40002);
    graphData.setLabel("Test2");
    graphData.setUnsigned(true);
    graphListRef.append(graphData);


    QList<GraphData> graphList;
    QModelIndex modelIdx;

    // Start all unchecked
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 0);

    // Check first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 1);
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    EXPECT_EQ(graphList[0].isActive(), graphListRef[0].isActive());
    EXPECT_EQ(graphList[0].registerAddress(), graphListRef[0].registerAddress());
    EXPECT_EQ(graphList[0].label(), graphListRef[0].label());
    EXPECT_EQ(graphList[0].isUnsigned(), graphListRef[0].isUnsigned());

    // Check second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Checked), Qt::CheckStateRole), true);

    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 2);
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    EXPECT_EQ(graphList[0].isActive(), graphListRef[0].isActive());
    EXPECT_EQ(graphList[0].registerAddress(), graphListRef[0].registerAddress());
    EXPECT_EQ(graphList[0].label(), graphListRef[0].label());
    EXPECT_EQ(graphList[0].isUnsigned(), graphListRef[0].isUnsigned());

    EXPECT_EQ(graphList[1].isActive(), graphListRef[1].isActive());
    EXPECT_EQ(graphList[1].registerAddress(), graphListRef[1].registerAddress());
    EXPECT_EQ(graphList[1].label(), graphListRef[1].label());
    EXPECT_EQ(graphList[1].isUnsigned(), graphListRef[1].isUnsigned());

    // Uncheck first
    modelIdx = pMbcRegisterModel->index(0, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 1);
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());

    graphList = pMbcRegisterModel->selectedRegisterList();
    EXPECT_EQ(graphList[0].isActive(), graphListRef[1].isActive());
    EXPECT_EQ(graphList[0].registerAddress(), graphListRef[1].registerAddress());
    EXPECT_EQ(graphList[0].label(), graphListRef[1].label());
    EXPECT_EQ(graphList[0].isUnsigned(), graphListRef[1].isUnsigned());


    // Uncheck second
    modelIdx = pMbcRegisterModel->index(1, cColumnSelected);
    EXPECT_EQ(pMbcRegisterModel->setData(modelIdx, QVariant(Qt::Unchecked), Qt::CheckStateRole), true);

    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), 0);
    EXPECT_EQ(pMbcRegisterModel->selectedRegisterCount(), pMbcRegisterModel->selectedRegisterList().size());
}
