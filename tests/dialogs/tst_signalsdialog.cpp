
#include "tst_signalsdialog.h"

#include "../models/devicelisthelpers.h"
#include "dialogs/signaleditorpanel.h"
#include "dialogs/signalsdialog.h"
#include "models/graphdatamodel.h"

#include <QAbstractProxyModel>
#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTableView>
#include <QTest>
#include <QTimer>

/*!
 * \brief Builds a minimal "modbus" data-point schema with non-empty defaults, so the panel
 * builds a reference when the dialog opens.
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
    _answeredQuestions = 0;
    DeviceListHelpers::clearDevices(&_settingsModel);
    _settingsModel.setAdapterDataPointSchema("modbus", buildTestRegisterSchema());

    _pGraphDataModel = new GraphDataModel();
    _pMockDataPointManager = new MockDataPointManager(QStringLiteral("modbus"), &_settingsModel);
    _pMockDataPointManager->mockReady = false;
    _pMockHub = new MockAdapterHub();
    _pMockHub->addManager(QStringLiteral("modbus"), _pMockDataPointManager);
}

void TestSignalsDialog::cleanup()
{
    delete _pDialog;
    _pDialog = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pMockDataPointManager;
    _pMockDataPointManager = nullptr;
    delete _pMockHub;
    _pMockHub = nullptr;
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
    auto* pProxy = qobject_cast<QAbstractProxyModel*>(pView->model());
    Q_ASSERT(pProxy != nullptr);
    return qobject_cast<GraphDataModel*>(pProxy->sourceModel());
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

/*!
 * \brief Answer the next question with \a button.
 *
 * The question runs its own event loop, which delivers the timer while it is open.
 */
void TestSignalsDialog::answerNextQuestion(QMessageBox::StandardButton button)
{
    QTimer::singleShot(0, this, [this, button]() { clickInActiveMessageBox(button); });
}

//! Click \a button in the message box that is open, if any, and count the answer.
void TestSignalsDialog::clickInActiveMessageBox(QMessageBox::StandardButton button)
{
    auto* pBox = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
    if (pBox == nullptr)
    {
        return;
    }

    _answeredQuestions++;
    pBox->button(button)->click();
}

//! Click Cancel in the button box.
void TestSignalsDialog::clickCancel()
{
    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Cancel)->click();
}

void TestSignalsDialog::cancelWithoutChangesClosesWithoutQuestion()
{
    seedSignals(2);
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QSignalSpy spyFinished(_pDialog, &QDialog::finished);
    answerNextQuestion(QMessageBox::Cancel);

    clickCancel();
    QTest::qWait(0); /* deliver the answer timer, which finds no question */

    QCOMPARE(spyFinished.count(), 1);
    QCOMPARE(_answeredQuestions, 0);
}

void TestSignalsDialog::cancelQuestionKeepsDialogOpen()
{
    seedSignals(2);
    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    QSignalSpy spyFinished(_pDialog, &QDialog::finished);
    draftModel()->setLabel(GraphIdx(0), QStringLiteral("renamed"));
    answerNextQuestion(QMessageBox::Cancel);

    clickCancel();

    QCOMPARE(_answeredQuestions, 1);
    QCOMPARE(spyFinished.count(), 0);
    QCOMPARE(draftModel()->label(GraphIdx(0)), QStringLiteral("renamed"));
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
    QSignalSpy spyFinished(_pDialog, &QDialog::finished);
    answerNextQuestion(QMessageBox::Discard);

    clickCancel();

    QCOMPARE(_answeredQuestions, 1);
    QCOMPARE(spyFinished.count(), 1);
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
    QSignalSpy spyFinished(_pDialog, &QDialog::finished);
    answerNextQuestion(QMessageBox::Discard);

    QTest::keyClick(_pDialog, Qt::Key_Escape);

    QCOMPARE(_answeredQuestions, 1);
    QCOMPARE(spyFinished.count(), 1);
    QCOMPARE(_pDialog->result(), int(QDialog::Rejected));
    QVERIFY(_pGraphDataModel->isActive(GraphIdx(0)));
    QCOMPARE(_pGraphDataModel->dataSeries(GraphIdx(0))->size(), 3);
}

//! An edit still open in a table cell counts as a change when the window is closed.
void TestSignalsDialog::closeWithOpenCellEditAsksQuestion()
{
    openWithReadyAdapter(1);
    _pDialog->activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(_pDialog));
    if (QApplication::activeWindow() != _pDialog)
    {
        /* Committing the edit relies on a focus change, which only reaches an active window */
        QSKIP("The platform does not activate windows");
    }
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    pView->edit(pView->model()->index(0, GraphDataModel::column::TEXT));
    auto* pEditor = qobject_cast<QLineEdit*>(QApplication::focusWidget());
    QVERIFY(pEditor != nullptr);
    pEditor->setText(QStringLiteral("renamed"));
    answerNextQuestion(QMessageBox::Cancel);

    _pDialog->close();

    QCOMPARE(_answeredQuestions, 1);
    QVERIFY(_pDialog->isVisible());
    QCOMPARE(draftModel()->label(GraphIdx(0)), QStringLiteral("renamed"));
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
    _pMockDataPointManager->mockReady = true;
    seedSignals(signalCount);

    _pDialog = new SignalsDialog(_pGraphDataModel, &_settingsModel, _pMockHub);
    _pDialog->show();
    QVERIFY(QTest::qWaitForWindowExposed(_pDialog));
}

//! Answer the panel's pending build request with \a expression and click Add signal.
void TestSignalsDialog::addSignalThroughPanel(const QString& expression)
{
    _pMockDataPointManager->injectBuildResult(expression);
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
    const QRect lastRowRect = pView->visualRect(pView->model()->index(40, 0));
    QVERIFY(pView->viewport()->rect().intersects(lastRowRect));
}

void TestSignalsDialog::enterInPanelDoesNotCloseDialog()
{
    openWithReadyAdapter(0);
    _pMockDataPointManager->injectBuildResult(QStringLiteral("${h0}"));
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
    const QString nameBefore = pName->text();

    QTest::keyClick(pName, Qt::Key_Delete);

    QCOMPARE(draftModel()->size(), 2);
    QCOMPARE(pName->text(), nameBefore.mid(1));
}

void TestSignalsDialog::cancelDiscardsPanelAdds()
{
    openWithReadyAdapter(1);
    addSignalThroughPanel(QStringLiteral("${h0}"));
    QCOMPARE(draftModel()->size(), 2);
    answerNextQuestion(QMessageBox::Discard);

    clickCancel();

    QCOMPARE(_answeredQuestions, 1);
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

//! The panel opens wider than its minimum, so the address fields are not cramped.
void TestSignalsDialog::panelOpensWiderThanItsMinimum()
{
    openWithReadyAdapter(3);

    auto* pScrollArea = _pDialog->findChild<QScrollArea*>("panelScrollArea");
    QVERIFY(pScrollArea->width() > pScrollArea->minimumWidth());
}

//! Extra dialog width goes to the table; the panel keeps its width.
void TestSignalsDialog::widerDialogWidensOnlyTheTable()
{
    openWithReadyAdapter(3);

    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    auto* pScrollArea = _pDialog->findChild<QScrollArea*>("panelScrollArea");
    const int viewWidth = pView->width();
    const int panelWidth = pScrollArea->width();

    _pDialog->resize(_pDialog->width() + 200, _pDialog->height());

    QTRY_VERIFY(pView->width() > viewWidth);
    QCOMPARE(pScrollArea->width(), panelWidth);
}

//! Expressions of the draft signals, in table order.
QStringList TestSignalsDialog::draftExpressions() const
{
    QStringList expressions;
    for (qint32 idx = 0; idx < draftModel()->size(); idx++)
    {
        expressions.append(draftModel()->expression(GraphIdx(idx)));
    }
    return expressions;
}

void TestSignalsDialog::dataPointColumnShowsDescription()
{
    openWithReadyAdapter(1);
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    const QModelIndex dataPointIndex = pView->model()->index(0, GraphDataModel::column::COUNT);

    QCOMPARE(pView->model()->headerData(GraphDataModel::column::COUNT, Qt::Horizontal).toString(),
             QStringLiteral("Data point"));
    QCOMPARE(dataPointIndex.data().toString(), QStringLiteral("${40001}"));
    QCOMPARE(_pMockDataPointManager->describeCalls, QStringList({ QStringLiteral("${40001}") }));

    _pMockDataPointManager->injectDescribeResult(0, QStringLiteral("Holding register 40001"));

    QCOMPARE(dataPointIndex.data().toString(), QStringLiteral("Holding register 40001"));
}

//! Dragging a row by its Data point cell moves the signal, and OK applies the new order.
void TestSignalsDialog::dragFromDataPointColumnReordersSignals()
{
    openWithReadyAdapter(3);
    QAbstractItemModel* pModel = _pDialog->findChild<QTableView*>("signalView")->model();

    QScopedPointer<QMimeData> pData(pModel->mimeData({ pModel->index(0, GraphDataModel::column::COUNT) }));
    QVERIFY(pData);
    QVERIFY(pModel->canDropMimeData(pData.data(), Qt::MoveAction, 3, -1, QModelIndex()));
    pModel->dropMimeData(pData.data(), Qt::MoveAction, 3, -1, QModelIndex());

    const QStringList expected = { QStringLiteral("${40002}"), QStringLiteral("${40003}"), QStringLiteral("${40001}") };
    QCOMPARE(draftExpressions(), expected);

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok)->click();

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), expected.at(0));
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(2)), expected.at(2));
}

void TestSignalsDialog::clickOnDataPointColumnSelectsRowForRemove()
{
    openWithReadyAdapter(3);
    auto* pView = _pDialog->findChild<QTableView*>("signalView");

    const QRect cellRect = pView->visualRect(pView->model()->index(1, GraphDataModel::column::COUNT));
    QTest::mouseClick(pView->viewport(), Qt::LeftButton, Qt::NoModifier, cellRect.center());

    const QModelIndexList selectedRows = pView->selectionModel()->selectedRows();
    QCOMPARE(selectedRows.size(), 1);
    QCOMPARE(selectedRows.first().row(), 1);

    QTest::mouseClick(_pDialog->findChild<QPushButton*>("btnRemove"), Qt::LeftButton);

    QCOMPARE(draftExpressions(), QStringList({ QStringLiteral("${40001}"), QStringLiteral("${40003}") }));
}

//! Remove the row \a row through Remove selected.
void TestSignalsDialog::removeRowThroughButton(int row)
{
    _pDialog->findChild<QTableView*>("signalView")->selectRow(row);
    QTest::mouseClick(_pDialog->findChild<QPushButton*>("btnRemove"), Qt::LeftButton);
}

//! The selected rows of the signal table.
QList<int> TestSignalsDialog::selectedRows() const
{
    QList<int> rows;
    const QModelIndexList indexes = _pDialog->findChild<QTableView*>("signalView")->selectionModel()->selectedRows();
    for (const QModelIndex& index : indexes)
    {
        rows.append(index.row());
    }
    return rows;
}

void TestSignalsDialog::removeMiddleRowSelectsNextRow()
{
    openWithReadyAdapter(3);

    removeRowThroughButton(1);

    QCOMPARE(draftExpressions(), QStringList({ QStringLiteral("${40001}"), QStringLiteral("${40003}") }));
    QCOMPARE(selectedRows(), QList<int>({ 1 }));
}

void TestSignalsDialog::removeLastRowSelectsNewLastRow()
{
    openWithReadyAdapter(3);

    removeRowThroughButton(2);

    QCOMPARE(draftExpressions(), QStringList({ QStringLiteral("${40001}"), QStringLiteral("${40002}") }));
    QCOMPARE(selectedRows(), QList<int>({ 1 }));
}

void TestSignalsDialog::removeOnlyRowLeavesNoSelection()
{
    openWithReadyAdapter(1);

    removeRowThroughButton(0);

    QCOMPARE(draftModel()->size(), 0);
    QVERIFY(selectedRows().isEmpty());
}

/*!
 * \brief Select the first signal, change its address in the panel and answer the build request.
 * \param address        The address to enter.
 * \param builtReference The reference the adapter builds for it.
 */
void TestSignalsDialog::editFirstSignalAddressThroughPanel(int address, const QString& builtReference)
{
    _pDialog->findChild<QTableView*>("signalView")->selectRow(0);

    const qsizetype describeIdx = _pMockDataPointManager->describeCalls.indexOf(draftModel()->expression(GraphIdx(0)));
    QVERIFY(describeIdx >= 0);
    _pMockDataPointManager->injectDescribeResult(static_cast<int>(describeIdx), QStringLiteral("described"));

    auto* pAddressSpin = panel()->findChild<QSpinBox*>();
    QVERIFY(pAddressSpin != nullptr);
    QVERIFY(pAddressSpin->isEnabled());
    pAddressSpin->setValue(address);
    _pMockDataPointManager->injectBuildResult(builtReference);

    QCOMPARE(draftModel()->expression(GraphIdx(0)), builtReference);
}

void TestSignalsDialog::selectingRowEditsSignalInPanel()
{
    openWithReadyAdapter(2);
    draftModel()->setLabel(GraphIdx(1), QStringLiteral("Second"));
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    auto* pName = panel()->findChild<QLineEdit*>("lineName");
    auto* pTitle = panel()->findChild<QLabel*>("lblTitle");

    pView->selectRow(1);

    QCOMPARE(pTitle->text(), QStringLiteral("Edit signal"));
    QCOMPARE(pName->text(), QStringLiteral("Second"));

    pView->clearSelection();

    QCOMPARE(pTitle->text(), QStringLiteral("New signal"));
    QCOMPARE(pName->text(), QString());

    pView->selectRow(0);
    QTest::mouseClick(_pDialog->findChild<QPushButton*>("btnAdd"), Qt::LeftButton);

    QCOMPARE(pTitle->text(), QStringLiteral("New signal"));
}

void TestSignalsDialog::cancelDiscardsPanelEdits()
{
    openWithReadyAdapter(1);
    editFirstSignalAddressThroughPanel(5, QStringLiteral("${h5}"));
    answerNextQuestion(QMessageBox::Discard);

    clickCancel();

    QCOMPARE(_answeredQuestions, 1);
    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${40001}"));
}

void TestSignalsDialog::okAppliesPanelEdits()
{
    openWithReadyAdapter(1);
    editFirstSignalAddressThroughPanel(5, QStringLiteral("${h5}"));

    _pDialog->findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok)->click();

    QCOMPARE(_pGraphDataModel->expression(GraphIdx(0)), QStringLiteral("${h5}"));
}

//! The panel keeps editing the same signal after it was dragged to another row.
void TestSignalsDialog::editModeFollowsMovedRow()
{
    openWithReadyAdapter(3);
    auto* pView = _pDialog->findChild<QTableView*>("signalView");
    pView->selectRow(0);

    QAbstractItemModel* pModel = pView->model();
    QScopedPointer<QMimeData> pData(pModel->mimeData({ pModel->index(0, GraphDataModel::column::TEXT) }));
    pModel->dropMimeData(pData.data(), Qt::MoveAction, 3, -1, QModelIndex());
    QCOMPARE(draftModel()->expression(GraphIdx(2)), QStringLiteral("${40001}"));

    auto* pName = panel()->findChild<QLineEdit*>("lineName");
    pName->selectAll();
    QTest::keyClicks(pName, "Moved");

    QCOMPARE(draftModel()->label(GraphIdx(2)), QStringLiteral("Moved"));
}

QTEST_MAIN(TestSignalsDialog)
