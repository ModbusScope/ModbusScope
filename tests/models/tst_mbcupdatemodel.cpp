
#include "tst_mbcupdatemodel.h"

#include "models/graphdatamodel.h"
#include "models/mbcupdatemodel.h"
#include "models/settingsmodel.h"

#include <QTest>
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
    MbcUpdateModel* pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

    QCOMPARE(pMbcUpdateModel->rowCount(), 0);

    _pGraphDataModel->add();

    QVERIFY(pMbcUpdateModel->rowCount() == 1);
}

void TestMbcUpdateModel::noUpdate()
{
    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(0, "${40001}");
    _pGraphDataModel->setLabel(0, "Register 40001");

    MbcUpdateModel* pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

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

    MbcUpdateModel* pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

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

    MbcUpdateModel* pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

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

    MbcUpdateModel* pMbcUpdateModel = new MbcUpdateModel(_pGraphDataModel);

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

QTEST_GUILESS_MAIN(TestMbcUpdateModel)
