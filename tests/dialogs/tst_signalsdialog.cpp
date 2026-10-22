
#include "tst_signalsdialog.h"

#include "../models/devicelisthelpers.h"
#include "dialogs/signaleditorpanel.h"
#include "dialogs/signalsdialog.h"
#include "models/graphdatamodel.h"

#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalSpy>
#include <QTableView>
#include <QTest>

namespace {

//! Build requests the add form sends when the dialog opens: one for its default reference.
constexpr qsizetype cAddFormBuildCalls = 1;

} // namespace

/*!
 * \brief Builds a minimal "modbus" data-point schema with non-empty defaults.
 *
 * SignalsDialog::requestDefaultExpression() bails out early when "defaults" is
 * empty, so the schema needs at least one default field for the flow under test
 * to reach buildExpression().
 */
QJsonObject TestSignalsDialog::buildTestRegisterSchema()
{
    QJsonObject addressField;
    addressField["type"] = QStringLiteral("integer");
    addressField["title"] = QStringLiteral("Address");

    QJsonObject properties;
    properties["address"] = addressField;

    QJsonObject addressSchema;
    addressSchema["type"] = QStringLiteral("object");
    addressSchema["properties"] = properties;

    QJsonObject defaults;
    defaults["address"] = 0;
    defaults["dataType"] = QStringLiteral("16b");

    QJsonObject schema;
    schema["addressSchema"] = addressSchema;
    schema["defaults"] = defaults;
    return schema;
}

void TestSignalsDialog::init()
{
    DeviceListHelpers::clearDevices(&_settingsModel);
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pGraphDataModel = new GraphDataModel();
    _pMockAdapterManager = new MockAdapterManager(&_settingsModel);
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockAdapterManager);
}

void TestSignalsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pMockAdapterManager;
    _pMockAdapterManager = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
}

/*!
 * \brief Reproduces F3: after device 1 is deleted, the default register must target the
 * lowest remaining device id, not the buildExpression() sentinel that resolves to device 1.
 */
void TestSignalsDialog::defaultSignalUsesLowestConfiguredDeviceId()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 5, QStringLiteral("modbus"));

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

//! Confirms the fix doesn't disturb the already-correct case where device 1 is present.
void TestSignalsDialog::defaultSignalUsesDeviceOneWhenPresent()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    DeviceListHelpers::seedDevice(&_settingsModel, 3, QStringLiteral("modbus"));

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(1));
}

//! With no devices configured, SignalsDialog must not wire up a default-expression manager at all.
void TestSignalsDialog::defaultSignalSkipsBuildExpressionWhenNoDevices()
{
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), 0);
}

/*!
 * \brief sessionStarted() fires only once per session. When the dialog is opened after the
 * adapter session is already active (the common case in practice), that emission is already
 * in the past, so SignalsDialog must request the default expression directly instead of
 * waiting for a signal that will never come again.
 */
void TestSignalsDialog::defaultSignalRequestedImmediatelyWhenSessionAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    /* The add form also builds its default reference when the dialog opens */
    QCOMPARE(_pMockAdapterManager->buildCalls.count(), cAddFormBuildCalls + 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief AdapterClient::buildExpression() also accepts the AWAITING_CONFIG state (adapter
 * described, schema available, session not yet started) — SignalsDialog must not wait for
 * sessionStarted() in that state either, since the schema is already known.
 */
void TestSignalsDialog::defaultSignalRequestedImmediatelyWhenAdapterAlreadyReady()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockReady = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    /* The add form also builds its default reference when the dialog opens */
    QCOMPARE(_pMockAdapterManager->buildCalls.count(), cAddFormBuildCalls + 1);
    QCOMPARE(_pMockAdapterManager->buildCalls.last().deviceId, deviceId_t(2));
}

/*!
 * \brief The immediate request for an already-active session must not prevent a later, genuinely
 * new session from also triggering a request.
 */
void TestSignalsDialog::defaultSignalRequestedAgainWhenSessionRestartsAfterAlreadyActive()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 2, QStringLiteral("modbus"));
    _pMockAdapterManager->mockActive = true;

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QCOMPARE(_pMockAdapterManager->buildCalls.count(), cAddFormBuildCalls + 1);

    _pMockAdapterManager->injectSessionStarted();

    QCOMPARE(_pMockAdapterManager->buildCalls.count(), cAddFormBuildCalls + 2);
}

/*!
 * \brief Builds a describe response reporting a data point limit in its capabilities.
 * \param maxRegisters The limit to report, or a negative value to leave the key out entirely
 *        (which is how the adapter reports that it enforces no limit).
 */
QJsonObject TestSignalsDialog::buildDescribeWithMaxRegisters(int maxRegisters)
{
    QJsonObject capabilities;
    if (maxRegisters >= 0)
    {
        capabilities["maxRegisters"] = maxRegisters;
    }

    QJsonObject describeResult;
    describeResult["name"] = QStringLiteral("modbus");
    describeResult["capabilities"] = capabilities;
    return describeResult;
}

//! Add \a count active signals, each reading its own data point from the default device.
void TestSignalsDialog::seedSignals(int count)
{
    for (int idx = 0; idx < count; idx++)
    {
        _pGraphDataModel->add();
        _pGraphDataModel->setExpression(GraphIdx(_pGraphDataModel->size() - 1), QString("${%1}").arg(40001 + idx));
    }
}

void TestSignalsDialog::dataPointLimitWarningHiddenWhenWithinLimit()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(5);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(warningLabel->isHidden());
}

void TestSignalsDialog::dataPointLimitWarningShownWhenExceeded()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(6);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());
    QVERIFY(warningLabel->text().contains("modbus"));
    QVERIFY(warningLabel->text().contains("5"));
    QVERIFY(warningLabel->text().contains("6"));
    QVERIFY(warningLabel->text().contains("Request License"));
}

/*!
 * \brief Deactivating a signal drops its data point from the session, so the warning must
 * disappear again while the dialog stays open.
 */
void TestSignalsDialog::dataPointLimitWarningHiddenWhenSignalDeactivated()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(5));
    seedSignals(6);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(!warningLabel->isHidden());

    draftModel()->setActive(GraphIdx(0), false);

    QVERIFY(warningLabel->isHidden());
}

//! A licensed adapter reports no maxRegisters at all, so no number of signals may warn.
void TestSignalsDialog::dataPointLimitWarningHiddenWhenAdapterReportsNoLimit()
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _settingsModel.updateAdapterFromDescribe(QStringLiteral("modbus"), buildDescribeWithMaxRegisters(-1));
    seedSignals(20);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* warningLabel = _pDialog->findChild<QLabel*>("dataPointLimitWarningLabel");
    QVERIFY(warningLabel != nullptr);
    QVERIFY(warningLabel->isHidden());
}

//! The model the dialog's table edits.
GraphDataModel* TestSignalsDialog::draftModel() const
{
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    Q_ASSERT(pView != nullptr);
    return qobject_cast<GraphDataModel*>(pView->model());
}

//! Give every signal in the source model three samples, as after a stopped log.
void TestSignalsDialog::seedSamples()
{
    for (qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        _pGraphDataModel->mutableDataSeries(GraphIdx(idx))->setSamples({ 0, 1, 2 }, { 10, 11, 12 });
    }
}

//! Add a signal, remove the second one, deactivate the first and edit the expression of the third.
void TestSignalsDialog::editDraft()
{
    GraphData data;
    data.setLabel("added");
    data.setExpression("${40100}");
    draftModel()->add(data);
    draftModel()->removeSignals({ GraphIdx(1) });
    draftModel()->setActive(GraphIdx(0), false);
    draftModel()->setExpression(GraphIdx(1), "${40200}");
}

void TestSignalsDialog::buttonBoxHasOkAndCancel()
{
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);

    auto* pButtonBox = _pDialog->findChild<QDialogButtonBox*>("buttonBox");
    QVERIFY(pButtonBox != nullptr);
    QVERIFY(pButtonBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(pButtonBox->button(QDialogButtonBox::Cancel) != nullptr);
}

void TestSignalsDialog::tableEditsDraftNotSource()
{
    seedSignals(2);
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QVERIFY(draftModel() != _pGraphDataModel);

    draftModel()->setData(draftModel()->index(0, GraphDataModel::column::TEXT), "renamed", Qt::EditRole);

    QCOMPARE(draftModel()->label(GraphIdx(0)), QString("renamed"));
    QVERIFY(_pGraphDataModel->label(GraphIdx(0)) != QString("renamed"));
}

void TestSignalsDialog::cancelDiscardsChanges()
{
    seedSignals(3);
    seedSamples();
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    editDraft();

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Cancel)->click();

    QCOMPARE(_pDialog->result(), int(QDialog::Rejected));
    QCOMPARE(_pGraphDataModel->size(), 3);
    for (qint32 idx = 0; idx < 3; idx++)
    {
        QVERIFY(_pGraphDataModel->isActive(GraphIdx(idx)));
        QCOMPARE(_pGraphDataModel->expression(GraphIdx(idx)), QString("${%1}").arg(40001 + idx));
        QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(idx))->size(), 3);
    }
}

void TestSignalsDialog::escapeDiscardsChanges()
{
    seedSignals(1);
    seedSamples();
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    draftModel()->setActive(GraphIdx(0), false);

    QTest::keyClick(_pDialog, Qt::Key_Escape);

    QCOMPARE(_pDialog->result(), int(QDialog::Rejected));
    QVERIFY(_pGraphDataModel->isActive(GraphIdx(0)));
    QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(0))->size(), 3);
}

void TestSignalsDialog::okAppliesChanges()
{
    seedSignals(4);
    seedSamples();
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    editDraft();

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok)->click();

    QCOMPARE(_pDialog->result(), int(QDialog::Accepted));
    QCOMPARE(_pGraphDataModel->size(), 4);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QString("${40001}"));
    QVERIFY(!_pGraphDataModel->isActive(GraphIdx(0)));
    QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(0))->size(), 0);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(1)), QString("${40200}"));
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(2)), QString("${40004}"));
    QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(2))->size(), 3);
    QCOMPARE(_pGraphDataModel->label(GraphIdx(3)), QString("added"));
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(3)), QString("${40100}"));
}

void TestSignalsDialog::okWithoutChangesLeavesSourceUntouched()
{
    seedSignals(2);
    seedSamples();
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QSignalSpy spyDataChanged(_pGraphDataModel, &GraphDataModel::dataChanged);
    QSignalSpy spyInserted(_pGraphDataModel, &GraphDataModel::rowsInserted);
    QSignalSpy spyRemoved(_pGraphDataModel, &GraphDataModel::rowsRemoved);

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok)->click();

    QCOMPARE(spyDataChanged.count(), 0);
    QCOMPARE(spyInserted.count(), 0);
    QCOMPARE(spyRemoved.count(), 0);
    QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(0))->size(), 3);
}

//! The signal editor panel next to the table.
SignalEditorPanel* TestSignalsDialog::panel() const
{
    auto* pPanel = _pDialog->findChild<SignalEditorPanel*>();
    Q_ASSERT(pPanel != nullptr);
    return pPanel;
}

//! Seed \a signalCount signals on device 1, open the dialog with its adapter ready and show it.
void TestSignalsDialog::openWithReadyAdapter(int signalCount)
{
    DeviceListHelpers::seedDevice(&_settingsModel, 1, QStringLiteral("modbus"));
    _pMockAdapterManager->mockReady = true;
    seedSignals(signalCount);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    _pDialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pDialog));
}

//! Answer the panel's pending build request with \a expression and click Add signal.
void TestSignalsDialog::addSignalThroughPanel(const QString& expression)
{
    _pMockAdapterManager->injectBuildExpressionResult(expression);
    QTest::mouseClick(panel()->findChild<QPushButton*>("btnAdd"), Qt::LeftButton);
}

void TestSignalsDialog::addButtonClearsSelectionAndFocusesName()
{
    openWithReadyAdapter(3);
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    pView->selectRow(1);

    QTest::mouseClick(_pDialog->findChild<QPushButton*>("btnAdd"), Qt::LeftButton);

    QVERIFY(!pView->selectionModel()->hasSelection());
    /* The window's focus widget does not depend on the window being active */
    QCOMPARE(_pDialog->focusWidget(), panel()->findChild<QLineEdit*>("lineName"));
}

//! A signal added through the panel is scrolled into view, but not selected: selection is for editing.
void TestSignalsDialog::addedSignalScrolledIntoViewNotSelected()
{
    openWithReadyAdapter(40);
    auto* pView = _pDialog->findChild<QTableView*>("signalView");

    addSignalThroughPanel(QStringLiteral("${h0}"));

    QCOMPARE(draftModel()->size(), 41);
    QVERIFY(!pView->selectionModel()->hasSelection());
    const QRect lastRowRect = pView->visualRect(draftModel()->index(40, 0));
    QVERIFY(pView->viewport()->rect().intersects(lastRowRect));
}

void TestSignalsDialog::enterInPanelDoesNotCloseDialog()
{
    openWithReadyAdapter(0);
    _pMockAdapterManager->injectBuildExpressionResult(QStringLiteral("${h0}"));
    auto* pName = panel()->findChild<QLineEdit*>("lineName");
    pName->setFocus();

    QTest::keyClick(pName, Qt::Key_Return);

    QVERIFY(_pDialog->isVisible());
    QCOMPARE(draftModel()->size(), 1);
    QCOMPARE(_pGraphDataModel->size(), 0);
}

void TestSignalsDialog::deleteInNameFieldKeepsRows()
{
    openWithReadyAdapter(2);
    _pDialog->findChild<QTableView*>("signalView")->selectRow(0);
    auto* pName = panel()->findChild<QLineEdit*>("lineName");
    pName->setFocus();
    QTest::keyClicks(pName, "ab");
    QTest::keyClick(pName, Qt::Key_Home);

    QTest::keyClick(pName, Qt::Key_Delete);

    QCOMPARE(draftModel()->size(), 2);
    QCOMPARE(pName->text(), QStringLiteral("b"));
}

void TestSignalsDialog::cancelDiscardsPanelAdds()
{
    openWithReadyAdapter(1);
    addSignalThroughPanel(QStringLiteral("${h0}"));
    QCOMPARE(draftModel()->size(), 2);

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Cancel)->click();

    QCOMPARE(_pGraphDataModel->size(), 1);
}

//! At its minimum size the dialog still shows table and panel, and Add signal can be scrolled to.
void TestSignalsDialog::minimumSizeKeepsTableAndPanelUsable()
{
    openWithReadyAdapter(3);

    _pDialog->resize(_pDialog->minimumSize());
    QTRY_COMPARE(_pDialog->size(), _pDialog->minimumSize());

    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    auto* pScrollArea = _pDialog->findChild<QScrollArea*>("panelScrollArea");
    QVERIFY(pView->isVisible());
    QVERIFY(pScrollArea->isVisible());
    QVERIFY(pView->width() >= pView->minimumWidth());
    QVERIFY(pScrollArea->width() >= pScrollArea->minimumWidth());

    auto* pAddSignal = panel()->findChild<QPushButton*>("btnAdd");
    pScrollArea->ensureWidgetVisible(pAddSignal);
    const QRect addRect(pAddSignal->mapTo(pScrollArea->viewport(), QPoint(0, 0)), pAddSignal->size());
    QVERIFY(pScrollArea->viewport()->rect().contains(addRect));
}

QTEST_MAIN(TestSignalsDialog)
