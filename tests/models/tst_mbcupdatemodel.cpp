#include "tst_mbcupdatemodel.h"

#include "models/graphdatamodel.h"
#include "models/mbcupdatemodel.h"
#include "models/settingsmodel.h"

#include <QTest>
#include <memory>
#include <qtestcase.h>

void TestMbcUpdateModel::init()
{
    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
}

void TestMbcUpdateModel::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestMbcUpdateModel::rowCount()
{
    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QCOMPARE(pMbcUpdateModel->rowCount(), 0);

    _pGraphDataModel->add();

    QVERIFY(pMbcUpdateModel->rowCount() == 1);
}

void TestMbcUpdateModel::noUpdate()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("Register 40001");

    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QString actExpression =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::DisplayRole).toString();
    QVERIFY(actExpression.isEmpty());

    QString actText =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::DisplayRole).toString();
    QVERIFY(actText.isEmpty());

    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, 0), Qt::ToolTipRole).toString(), "");

    Qt::ItemFlags expFlags = Qt::NoItemFlags;
    QCOMPARE(pMbcUpdateModel->flags(modelIdx.sibling(0, 0)), expFlags);
}

void TestMbcUpdateModel::tooMuchDifference()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40002);
    mbcRegisterList[0].setName("Register 40002");

    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QString actExpression =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::DisplayRole).toString();
    QVERIFY(actExpression.isEmpty());

    QString actText =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::DisplayRole).toString();
    QVERIFY(actText.isEmpty());

    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, 0), Qt::ToolTipRole).toString(), "");
}

void TestMbcUpdateModel::updateText()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("Register");

    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QString actExpression =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::DisplayRole).toString();
    QVERIFY(actExpression.isEmpty());

    QString actText =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::DisplayRole).toString();
    QCOMPARE(actText, mbcRegisterList[0].name());

    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, 0), Qt::ToolTipRole).toString(), "Update of text detected");

    Qt::ItemFlags expFlags = Qt::ItemIsEnabled;
    QCOMPARE(pMbcUpdateModel->flags(modelIdx.sibling(0, 0)), expFlags);
}

void TestMbcUpdateModel::updateExpression()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40002);
    mbcRegisterList[0].setName("Register 40001");

    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QString actExpression =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::DisplayRole).toString();
    QCOMPARE(actExpression, mbcRegisterList[0].toExpression());

    QString actText =
      pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::DisplayRole).toString();
    QVERIFY(actText.isEmpty());

    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, 0), Qt::ToolTipRole).toString(),
             "Update of expression detected");

    Qt::ItemFlags expFlags = Qt::ItemIsEnabled;
    QCOMPARE(pMbcUpdateModel->flags(modelIdx.sibling(0, 0)), expFlags);
}

void TestMbcUpdateModel::columnCount()
{
    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QCOMPARE(pMbcUpdateModel->columnCount(), static_cast<int>(MbcUpdateModel::cColumnCnt));
}

void TestMbcUpdateModel::userRoleHiddenWhenNoUpdate()
{
    // When there is no pending update the UserRole should return "hidden"
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Reg");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("Reg");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);

    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::UserRole).toString(), "hidden");
    QCOMPARE(pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::UserRole).toString(), "hidden");
}

void TestMbcUpdateModel::userRoleVisibleWhenUpdatePresent()
{
    // When there is a text update the cColumnUpdateText UserRole should NOT return "hidden"
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Old Name");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("New Name");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);

    // cColumnUpdateText has a pending update — UserRole should NOT be "hidden"
    QVariant userRoleText = pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateText), Qt::UserRole);
    QVERIFY(userRoleText.toString() != "hidden");

    // cColumnUpdateExpression has no update — UserRole should be "hidden"
    QVariant userRoleExpr = pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnUpdateExpression), Qt::UserRole);
    QCOMPARE(userRoleExpr.toString(), "hidden");
}

void TestMbcUpdateModel::flagsNoUpdateIsDisabled()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Reg");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    // No MBC registers set — no update pending
    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QCOMPARE(pMbcUpdateModel->flags(modelIdx), Qt::NoItemFlags);
}

void TestMbcUpdateModel::flagsWithUpdateIsEnabled()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Old Label");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("New Label");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QVERIFY(pMbcUpdateModel->flags(modelIdx) & Qt::ItemIsEnabled);
}

void TestMbcUpdateModel::rowCountTracksGraphModel()
{
    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QCOMPARE(pMbcUpdateModel->rowCount(), 0);

    _pGraphDataModel->add();
    QCOMPARE(pMbcUpdateModel->rowCount(), 1);

    _pGraphDataModel->add();
    QCOMPARE(pMbcUpdateModel->rowCount(), 2);
}

void TestMbcUpdateModel::setMbcRegistersTriggersCheck()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Old Label");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    // First: no pending update
    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QCOMPARE(pMbcUpdateModel->flags(modelIdx), Qt::NoItemFlags);

    // After setMbcRegisters with a different name, update should be detected
    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("New Label");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QVERIFY(pMbcUpdateModel->flags(modelIdx) & Qt::ItemIsEnabled);
}

void TestMbcUpdateModel::updateTextToolTip()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40001);
    mbcRegisterList[0].setName("Different Name");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QCOMPARE(pMbcUpdateModel->data(modelIdx, Qt::ToolTipRole).toString(), "Update of text detected");
}

void TestMbcUpdateModel::updateExpressionToolTip()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QList<MbcRegisterData> mbcRegisterList(1);
    mbcRegisterList[0].setRegisterAddress(40002);
    mbcRegisterList[0].setName("Register 40001");
    pMbcUpdateModel->setMbcRegisters(mbcRegisterList);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);
    QCOMPARE(pMbcUpdateModel->data(modelIdx, Qt::ToolTipRole).toString(), "Update of expression detected");
}

void TestMbcUpdateModel::displayColumnsCurrentValues()
{
    // Verify that cColumnExpression and cColumnText show the current graph data values
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "MyRegister");

    std::unique_ptr<MbcUpdateModel> pMbcUpdateModel = std::make_unique<MbcUpdateModel>(_pGraphDataModel);

    QModelIndex modelIdx = pMbcUpdateModel->index(0, 0);

    QString currentExpr = pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnExpression), Qt::DisplayRole).toString();
    QVERIFY(!currentExpr.isEmpty());

    QString currentText = pMbcUpdateModel->data(modelIdx.sibling(0, MbcUpdateModel::cColumnText), Qt::DisplayRole).toString();
    QCOMPARE(currentText, QString("MyRegister"));
}

QTEST_GUILESS_MAIN(TestMbcUpdateModel)
