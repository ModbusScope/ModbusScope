
#include "tst_expressionsdialog.h"

#include "dialogs/expressionsdialog.h"
#include "models/device.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTest>

void TestExpressionsDialog::init()
{
    _pGraphDataModel = new GraphDataModel(this);
    _pGraphDataModel->add(GraphData());

    _pSettingsModel = new SettingsModel(this);
    _pSettingsModel->deviceSettings(Device::cFirstDeviceId)->setAdapterId("modbus");

    _pMockAdapterManager = new MockAdapterManager(QStringLiteral("modbus"), this);
    _pMockHub = new MockAdapterHub(this);
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);

    _pDialog = new ExpressionsDialog(_pGraphDataModel, GraphIdx(0), _pMockHub, _pSettingsModel);
}

void TestExpressionsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
}

/*!
 * \brief Register a second ("sim") adapter manager in the hub.
 * \return Pointer to the created sim mock manager (owned by the test object).
 */
MockAdapterManager* TestExpressionsDialog::addSimAdapter()
{
    auto* pSimManager = new MockAdapterManager(QStringLiteral("sim"), this);
    _pMockHub->addManager(QStringLiteral("sim"), pSimManager);
    return pSimManager;
}

/*!
 * \brief Recreate the dialog so it picks up hub changes made after init().
 *
 * Resets the modbus mock's help request counter so tests only observe requests
 * made by the recreated dialog.
 */
void TestExpressionsDialog::recreateDialog()
{
    delete _pDialog;
    _pMockAdapterManager->helpRequestCount = 0;
    _pDialog = new ExpressionsDialog(_pGraphDataModel, GraphIdx(0), _pMockHub, _pSettingsModel);
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

void TestExpressionsDialog::describeRoutedPerDeviceAdapter()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    MockAdapterManager* pSimManager = addSimAdapter();

    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001} + ${40002@2}"));

    /* First data point belongs to device 1 (modbus adapter) */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));
    QCOMPARE(pSimManager->describeCalls.size(), 0);

    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Modbus reg"));

    /* Second data point belongs to device 2 (sim adapter) */
    QCOMPARE(pSimManager->describeCalls.size(), 1);
    QCOMPARE(pSimManager->describeCalls[0], QStringLiteral("${40002@2}"));

    pSimManager->injectDescribeResult(QStringLiteral("Sim reg"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Modbus reg"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Sim reg"));

    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
}

void TestExpressionsDialog::describeSkipsUnavailableAdapter()
{
    /* Device 2 belongs to an adapter without a manager in the hub */
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");

    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001@2} + ${40002}"));

    /* Row 0 is skipped; the describe chain continues with row 1 on the modbus adapter */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40002}"));

    _pMockAdapterManager->injectDescribeResult(QStringLiteral("Modbus reg"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);

    /* Skipped row keeps the parser description */
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("${40001@2}, device id 2"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Modbus reg"));

    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
}

void TestExpressionsDialog::describeSkipsIdleAdapter()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    MockAdapterManager* pSimManager = addSimAdapter();
    pSimManager->mockIdle = true;

    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001@2} + ${40002}"));

    /* The idle sim adapter is skipped; only the modbus data point is described */
    QCOMPARE(pSimManager->describeCalls.size(), 0);
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40002}"));
}

void TestExpressionsDialog::helpComboHiddenWithSingleAdapter()
{
    auto* pHelpWidget = _pDialog->findChild<QWidget*>("widgetHelpAdapter");
    QVERIFY(pHelpWidget != nullptr);
    QVERIFY(!pHelpWidget->isVisibleTo(_pDialog));
}

void TestExpressionsDialog::helpComboVisibleWithTwoAdapters()
{
    addSimAdapter();
    recreateDialog();

    auto* pHelpWidget = _pDialog->findChild<QWidget*>("widgetHelpAdapter");
    QVERIFY(pHelpWidget != nullptr);
    QVERIFY(pHelpWidget->isVisibleTo(_pDialog));

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpAdapter");
    QVERIFY(pCombo != nullptr);
    QCOMPARE(pCombo->count(), 2);
    QCOMPARE(pCombo->itemData(0).toString(), QStringLiteral("modbus"));
    QCOMPARE(pCombo->itemData(1).toString(), QStringLiteral("sim"));
}

void TestExpressionsDialog::helpRoutedToSelectedAdapter()
{
    MockAdapterManager* pSimManager = addSimAdapter();
    recreateDialog();

    auto* pInfoLabel = _pDialog->findChild<QLabel*>("lblInfo");
    QVERIFY(pInfoLabel != nullptr);

    /* The dialog requested help from the initially selected (modbus) adapter */
    QCOMPARE(_pMockAdapterManager->helpRequestCount, 1);
    QCOMPARE(pSimManager->helpRequestCount, 0);

    _pMockAdapterManager->injectExpressionHelp(QStringLiteral("Modbus help"));
    QCOMPARE(pInfoLabel->text(), QStringLiteral("Modbus help"));

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpAdapter");
    QVERIFY(pCombo != nullptr);
    pCombo->setCurrentIndex(1);

    /* Selecting the sim adapter clears the help text and requests its help */
    QCOMPARE(pSimManager->helpRequestCount, 1);
    QCOMPARE(pInfoLabel->text(), QString());

    pSimManager->injectExpressionHelp(QStringLiteral("Sim help"));
    QCOMPARE(pInfoLabel->text(), QStringLiteral("Sim help"));

    /* A late response from the deselected modbus adapter is ignored */
    _pMockAdapterManager->injectExpressionHelp(QStringLiteral("Stale modbus help"));
    QCOMPARE(pInfoLabel->text(), QStringLiteral("Sim help"));
}

QTEST_MAIN(TestExpressionsDialog)
