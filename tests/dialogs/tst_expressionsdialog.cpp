
#include "tst_expressionsdialog.h"

#include "dialogs/expressionsdialog.h"
#include "models/datapointdescriptioncache.h"
#include "models/device.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "ui_expressionsdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
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
    _pDataPointService = new DataPointService(_pSettingsModel, _pMockHub, this);
    _pDescriptionCache = new DataPointDescriptionCache(_pDataPointService, this);

    _pDialog = new ExpressionsDialog(_pGraphDataModel, GraphIdx(0), _pMockHub, _pSettingsModel, _pDescriptionCache);
}

void TestExpressionsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
    delete _pDescriptionCache;
    _pDescriptionCache = nullptr;
    delete _pDataPointService;
    _pDataPointService = nullptr;
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
    _pDialog = new ExpressionsDialog(_pGraphDataModel, GraphIdx(0), _pMockHub, _pSettingsModel, _pDescriptionCache);
}

void TestExpressionsDialog::describeDispatch_singleAddress()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));

    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Holding register 40001"));

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

    /* Every data point is described at once */
    pEdit->setPlainText(QStringLiteral("${40001} + ${40002}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));
    QCOMPARE(_pMockAdapterManager->describeCalls[1], QStringLiteral("${40002}"));

    /* Replies in reverse order still land in their own row */
    _pMockAdapterManager->injectDescribeResult(2, QStringLiteral("Reg 2"));
    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Reg 1"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Reg 1"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Reg 2"));
}

//! A reply for a data point that is no longer in the expression changes no row.
void TestExpressionsDialog::describeDispatch_replyForRemovedReferenceIgnored()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);

    /* Expression changes before the reply arrives */
    pEdit->setPlainText(QStringLiteral("${40002}"));
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 2);
    QCOMPARE(_pMockAdapterManager->describeCalls[1], QStringLiteral("${40002}"));

    /* The late reply for the old expression is ignored */
    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Reg 1"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QVERIFY(pTable->item(0, 0)->text() != QStringLiteral("Reg 1"));

    _pMockAdapterManager->injectDescribeResult(2, QStringLiteral("Reg 2"));
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Reg 2"));
}

//! A data point that was described before is shown again without a new request.
void TestExpressionsDialog::describeDispatch_cachedAcrossExpressionChanges()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001}"));
    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Reg 1"));

    pEdit->setPlainText(QStringLiteral("${40001} * 2"));

    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Reg 1"));
}

//! A failed describe request leaves the parser description in its row.
void TestExpressionsDialog::describeDispatch_failureKeepsParserDescription()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001}"));
    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    const QString parserDescription = pTable->item(0, 0)->text();

    _pMockAdapterManager->injectDescribeFailure(1, QStringLiteral("The adapter did not respond"));
    QTest::qWait(0);

    QCOMPARE(pTable->item(0, 0)->text(), parserDescription);
}

void TestExpressionsDialog::describeRoutedPerDeviceAdapter()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    MockAdapterManager* pSimManager = addSimAdapter();

    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001} + ${40002@2}"));

    /* Device 1 belongs to the modbus adapter, device 2 to the sim adapter */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40001}"));
    QCOMPARE(pSimManager->describeCalls.size(), 1);
    QCOMPARE(pSimManager->describeCalls[0], QStringLiteral("${40002@2}"));

    /* Both managers number their requests from 1: each reply must only reach its own row */
    pSimManager->injectDescribeResult(1, QStringLiteral("Sim reg"));
    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Modbus reg"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Modbus reg"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Sim reg"));
}

void TestExpressionsDialog::describeSkipsUnavailableAdapter()
{
    /* Device 2 belongs to an adapter without a manager in the hub */
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");

    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001@2} + ${40002}"));

    /* Row 0 is skipped; row 1 is described by the modbus adapter */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40002}"));

    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Modbus reg"));

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

void TestExpressionsDialog::describeShowsInvalidDeviceMessage()
{
    /* Device 99 was never added to the settings model */
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("${40001@99} + ${40002}"));

    /* Row 0 is skipped without a describe call; row 1 is described */
    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
    QCOMPARE(_pMockAdapterManager->describeCalls[0], QStringLiteral("${40002}"));

    _pMockAdapterManager->injectDescribeResult(1, QStringLiteral("Modbus reg"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 0)->text(), QStringLiteral("Invalid device"));
    QCOMPARE(pTable->item(1, 0)->text(), QStringLiteral("Modbus reg"));

    QCOMPARE(_pMockAdapterManager->describeCalls.size(), 1);
}

void TestExpressionsDialog::helpComboHiddenWithSingleDevice()
{
    /* Use isHidden(): the surrounding info panel (widgetInfo) starts collapsed,
     * so visibility relative to the dialog is always false */
    auto* pHelpWidget = _pDialog->findChild<QWidget*>("widgetHelpDevice");
    QVERIFY(pHelpWidget != nullptr);
    QVERIFY(pHelpWidget->isHidden());
}

void TestExpressionsDialog::helpComboVisibleWithTwoDevices()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    addSimAdapter();
    recreateDialog();

    auto* pHelpWidget = _pDialog->findChild<QWidget*>("widgetHelpDevice");
    QVERIFY(pHelpWidget != nullptr);
    QVERIFY(!pHelpWidget->isHidden());

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpDevice");
    QVERIFY(pCombo != nullptr);
    QCOMPARE(pCombo->count(), 2);
    QCOMPARE(pCombo->itemData(0).value<deviceId_t>(), deviceId_t(1));
    QCOMPARE(pCombo->itemData(1).value<deviceId_t>(), deviceId_t(2));
}

void TestExpressionsDialog::helpComboShowsNameProtocolAndId()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    _pSettingsModel->deviceSettings(2)->setName(QStringLiteral("PLC-North"));
    addSimAdapter();
    recreateDialog();

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpDevice");
    QVERIFY(pCombo != nullptr);
    QCOMPARE(pCombo->count(), 2);
    /* Label format: "<device name> (<protocol>) [<device id>]" */
    QCOMPARE(pCombo->itemText(0), QStringLiteral("Device 1 (modbus) [1]"));
    QCOMPARE(pCombo->itemText(1), QStringLiteral("PLC-North (sim) [2]"));
}

void TestExpressionsDialog::helpRoutedToSelectedDevice()
{
    _pSettingsModel->deviceSettings(2)->setAdapterId("sim");
    MockAdapterManager* pSimManager = addSimAdapter();
    recreateDialog();

    auto* pInfoLabel = _pDialog->findChild<QLabel*>("lblInfo");
    QVERIFY(pInfoLabel != nullptr);

    /* The dialog requested help from the initially selected device's (modbus) adapter */
    QCOMPARE(_pMockAdapterManager->helpRequestCount, 1);
    QCOMPARE(pSimManager->helpRequestCount, 0);

    _pMockAdapterManager->injectExpressionHelp(QStringLiteral("Modbus help"));
    /* Label combines the core's generic expression info with the adapter's fragment,
     * wrapped in a single well-formed document */
    QVERIFY(pInfoLabel->text().startsWith(QStringLiteral("<html><head/><body>")));
    QVERIFY(pInfoLabel->text().endsWith(QStringLiteral("</body></html>")));
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Expression Information")));
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Modbus help")));
    QVERIFY(pInfoLabel->text().indexOf(QStringLiteral("Expression Information")) <
            pInfoLabel->text().indexOf(QStringLiteral("Modbus help")));

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpDevice");
    QVERIFY(pCombo != nullptr);
    pCombo->setCurrentIndex(1);

    /* Selecting the device on the sim adapter clears the help text and requests its help */
    QCOMPARE(pSimManager->helpRequestCount, 1);
    QCOMPARE(pInfoLabel->text(), QString());

    pSimManager->injectExpressionHelp(QStringLiteral("Sim help"));
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Sim help")));
    QVERIFY(!pInfoLabel->text().contains(QStringLiteral("Modbus help")));

    /* A late response from the deselected modbus adapter is ignored */
    _pMockAdapterManager->injectExpressionHelp(QStringLiteral("Stale modbus help"));
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Sim help")));
    QVERIFY(!pInfoLabel->text().contains(QStringLiteral("Stale modbus help")));
}

void TestExpressionsDialog::helpSwitchingBetweenDevicesOnSameAdapterSkipsRedundantRequest()
{
    /* Device 3 shares the modbus adapter with device 1 */
    _pSettingsModel->deviceSettings(3)->setAdapterId("modbus");
    recreateDialog();

    auto* pInfoLabel = _pDialog->findChild<QLabel*>("lblInfo");
    QVERIFY(pInfoLabel != nullptr);
    QCOMPARE(_pMockAdapterManager->helpRequestCount, 1);

    _pMockAdapterManager->injectExpressionHelp(QStringLiteral("Modbus help"));
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Modbus help")));

    auto* pCombo = _pDialog->findChild<QComboBox*>("cmbHelpDevice");
    QVERIFY(pCombo != nullptr);
    QCOMPARE(pCombo->count(), 2);
    pCombo->setCurrentIndex(1);

    /* Switching to the other device on the same adapter is a no-op: no new
     * request is issued and the already-shown help text is not cleared */
    QCOMPARE(_pMockAdapterManager->helpRequestCount, 1);
    QVERIFY(pInfoLabel->text().contains(QStringLiteral("Modbus help")));
}

//! A new data point starts with example value 1, so a division by it evaluates.
void TestExpressionsDialog::exampleInputDefaultsToOne()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);

    pEdit->setPlainText(QStringLiteral("100 / ${40001}"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    QCOMPARE(pTable->item(0, 1)->text(), QStringLiteral("1"));

    auto* pOutput = _pDialog->findChild<QLabel*>("lblOut");
    auto* pError = _pDialog->findChild<QLabel*>("lblError");
    QVERIFY(pOutput != nullptr);
    QVERIFY(pError != nullptr);
    QCOMPARE(pOutput->text(), QStringLiteral("100"));
    QCOMPARE(pError->text(), QString());
}

//! A hexadecimal example value is evaluated as its integer value.
void TestExpressionsDialog::exampleInputAcceptsHex()
{
    auto* pEdit = _pDialog->findChild<QPlainTextEdit*>("lineExpression");
    QVERIFY(pEdit != nullptr);
    pEdit->setPlainText(QStringLiteral("${40001}"));

    auto* pTable = _pDialog->findChild<QTableWidget*>("tblExpressionInput");
    QVERIFY(pTable != nullptr);
    pTable->item(0, 1)->setText(QStringLiteral("0x10"));

    auto* pOutput = _pDialog->findChild<QLabel*>("lblOut");
    QVERIFY(pOutput != nullptr);
    QCOMPARE(pOutput->text(), QStringLiteral("16"));
}

//! A syntax error disables Accept, and accepting anyway leaves the signal unchanged.
void TestExpressionsDialog::acceptDisabledOnSyntaxError()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();

    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001} +"));
    QVERIFY(!_pDialog->_pAcceptButton->isEnabled());

    _pDialog->handleAccept();

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Rejected));
}

//! An empty expression cannot be accepted.
void TestExpressionsDialog::acceptDisabledOnEmptyExpression()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QString());
    recreateDialog();

    QCOMPARE(_pDialog->_pUi->lineExpression->toPlainText(), QString());
    QVERIFY(!_pDialog->_pAcceptButton->isEnabled());
}

//! A reference to a device that does not exist disables Accept and says why.
void TestExpressionsDialog::acceptDisabledOnUnknownDevice()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001@99}"));

    QVERIFY(!_pDialog->_pAcceptButton->isEnabled());
    QCOMPARE(_pDialog->_pUi->lblError->text(), QStringLiteral("Unknown device in expression"));
}

//! An unknown device is reported even when an example value is also invalid.
void TestExpressionsDialog::unknownDeviceShownWithInvalidExampleValue()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001@99}"));
    _pDialog->_pUi->tblExpressionInput->item(0, 1)->setText(QStringLiteral("abc"));

    QVERIFY(!_pDialog->_pAcceptButton->isEnabled());
    QCOMPARE(_pDialog->_pUi->lblError->text(), QStringLiteral("Unknown device in expression"));
}

//! A division by a zero example value shows an error but does not block Accept.
void TestExpressionsDialog::acceptEnabledOnDivisionByZeroExample()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("100 / ${40001}"));
    _pDialog->_pUi->tblExpressionInput->item(0, 1)->setText(QStringLiteral("0"));

    QVERIFY(!_pDialog->_pUi->lblError->text().isEmpty());
    QVERIFY(_pDialog->_pAcceptButton->isEnabled());
}

//! An example value that is not a number does not block Accept.
void TestExpressionsDialog::acceptEnabledOnInvalidExampleValue()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001}"));
    _pDialog->_pUi->tblExpressionInput->item(0, 1)->setText(QStringLiteral("abc"));

    QVERIFY(_pDialog->_pAcceptButton->isEnabled());
}

//! Accept writes a valid expression to the signal and closes the dialog.
void TestExpressionsDialog::acceptAppliesValidExpression()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001} * 2"));

    QTest::mouseClick(_pDialog->_pAcceptButton, Qt::LeftButton);

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001} * 2"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Accepted));
}

//! The dialog has a standard Cancel button and an Accept button.
void TestExpressionsDialog::buttonBoxHasAcceptAndCancel()
{
    QCOMPARE(_pDialog->_pUi->buttonBox->buttonRole(_pDialog->_pAcceptButton), QDialogButtonBox::AcceptRole);
    QVERIFY(_pDialog->_pUi->buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
}

//! Enter in the example input table does not accept, even though Accept is the default button.
void TestExpressionsDialog::enterInExampleTableIgnored()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40002}"));
    _pDialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pDialog));

    _pDialog->_pUi->tblExpressionInput->setFocus();
    QTest::keyClick(_pDialog->_pUi->tblExpressionInput, Qt::Key_Return);

    QVERIFY(_pDialog->isVisible());
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
}

//! Enter on a focused Cancel button cancels instead of accepting.
void TestExpressionsDialog::enterOnCancelCancels()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40002}"));
    _pDialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pDialog));

    QPushButton* pCancelButton = _pDialog->_pUi->buttonBox->button(QDialogButtonBox::Cancel);
    pCancelButton->setFocus();
    QTest::keyClick(pCancelButton, Qt::Key_Return);

    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Rejected));
    QVERIFY(!_pDialog->isVisible());
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
}

//! Escape closes the dialog without changing the signal.
void TestExpressionsDialog::escapeCancels()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40002}"));
    _pDialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pDialog));

    QTest::keyClick(_pDialog->_pUi->lineExpression, Qt::Key_Escape);

    QVERIFY(!_pDialog->isVisible());
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
}

//! Cancel closes the dialog without changing the signal.
void TestExpressionsDialog::cancelLeavesExpressionUnchanged()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40002}"));

    QTest::mouseClick(_pDialog->_pUi->buttonBox->button(QDialogButtonBox::Cancel), Qt::LeftButton);

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Rejected));
}

//! Enter in the editor applies a valid expression and closes the dialog.
void TestExpressionsDialog::enterAcceptsValidExpression()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001} * 2"));

    QTest::keyClick(_pDialog->_pUi->lineExpression, Qt::Key_Return);

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001} * 2"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Accepted));
}

//! Enter does nothing while the expression has a syntax error.
void TestExpressionsDialog::enterIgnoredOnSyntaxError()
{
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40001}"));
    recreateDialog();
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001} +"));

    QTest::keyClick(_pDialog->_pUi->lineExpression, Qt::Key_Enter);

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Rejected));
    QCOMPARE(_pDialog->_pUi->lineExpression->toPlainText(), QStringLiteral("${40001} +"));
}

//! Shift+Enter neither accepts nor adds a line break.
void TestExpressionsDialog::shiftEnterIgnored()
{
    _pDialog->_pUi->lineExpression->setPlainText(QStringLiteral("${40001}"));
    _pDialog->_pUi->lineExpression->moveCursor(QTextCursor::End);

    QTest::keyClick(_pDialog->_pUi->lineExpression, Qt::Key_Return, Qt::ShiftModifier);

    QCOMPARE(_pDialog->_pUi->lineExpression->toPlainText(), QStringLiteral("${40001}"));
    QCOMPARE(_pDialog->result(), static_cast<int>(QDialog::Rejected));
}

//! A long expression wraps instead of scrolling sideways.
void TestExpressionsDialog::expressionEditorWraps()
{
    QCOMPARE(_pDialog->_pUi->lineExpression->lineWrapMode(), QPlainTextEdit::WidgetWidth);
    QCOMPARE(_pDialog->_pUi->lineExpression->verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
}

QTEST_MAIN(TestExpressionsDialog)
