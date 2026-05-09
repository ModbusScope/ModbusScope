
#include "tst_expressionsdialog.h"

#include "dialogs/expressionsdialog.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"

#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTest>

void TestExpressionsDialog::init()
{
    _pGraphDataModel = new GraphDataModel(this);
    _pGraphDataModel->add(GraphData());
    _pMockAdapterManager = new MockAdapterManager(this);
    _pDialog = new ExpressionsDialog(_pGraphDataModel, 0, _pMockAdapterManager);
}

void TestExpressionsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
}

void TestExpressionsDialog::describeDispatch_singleAddress()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));

    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Holding register 40001"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Holding register 40001"));

    /* No further describe calls after last address */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
}

void TestExpressionsDialog::describeDispatch_multipleAddresses()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001} + ${40002}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));

    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Reg 1"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);
    QCOMPARE(_pMockAdapterManager->describeCalls[1], QStringLiteral("${40002}"));

    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Reg 2"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Reg 1"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Reg 2"));
}

void TestExpressionsDialog::describeDispatch_abortsOnExpressionChange()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    /* Start describe sequence for expression A */
    pEdit->setPlainText(QStringLiteral("${40001}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);

    /* Expression changes before result arrives: starts fresh sequence for B */
    pEdit->setPlainText(QStringLiteral("${40002}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);
    QCOMPARE(_pMockAdapterManager->describeCalls[1], QStringLiteral("${40002}"));

    /* Result arrives — must apply to expression B's row only */
    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Reg 2"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Reg 2"));

    /* Stale A connection was cancelled — no further calls triggered */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);
}

QTEST_MAIN(TestExpressionsDialog)
#include "tst_expressionsdialog.moc"
