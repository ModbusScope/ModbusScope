#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "communication/datapoint.h"
#include "customwidgets/datapointeditorwidget.h"
#include "customwidgets/devicecombobox.h"
#include "dialogs/expressionhighlighting.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "util/expressionreferences.h"
#include "util/expressionvalue.h"
#include "util/util.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTextCursor>
#include <QTextDocument>

using State = DataQuality::State;

namespace {

/*!
 * \brief Generic expression syntax info, common to every adapter.
 *
 * Adapter-specific addressing syntax is appended by handleExpressionHelpResult().
 */
QString genericExpressionInfoHtml()
{
    return QStringLiteral("<p><span style=\" font-size:12pt; font-weight:600;\">Expression Information</span></p>"
                          "<p>The most common binary operators are supported (|, &amp;, &lt;&lt;, &gt;&gt;). "
                          "The basic arithmetic operators are also supported (+, -, *, /, %, ^). "
                          "Hexadecimal numbers can be represented with the &quot;0x&quot; prefix. "
                          "Binary numbers can be represented with the &quot;0b&quot; prefix. "
                          "Floating point numbers are also supported. "
                          "Both a decimal point and a comma can be used as the decimal separator, "
                          "but an expression must use only one of them.</p>");
}

} // namespace

ExpressionsDialog::ExpressionsDialog(GraphDataModel* pGraphDataModel,
                                     GraphIdx idx,
                                     AdapterHub* pAdapterHub,
                                     SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     DataPointDescriptionCache* pDescriptionCache,
                                     QWidget* parent)
    : QDialog(parent),
      _pUi(new Ui::ExpressionsDialog),
      _pGraphDataModel(pGraphDataModel),
      _pAdapterHub(pAdapterHub),
      _pSettingsModel(pSettingsModel),
      _pDataPointService(pDataPointService),
      _pDescriptionCache(pDescriptionCache),
      _bUpdating(false)
{
    _pUi->setupUi(this);

    connect(_pDescriptionCache, &DataPointDescriptionCache::descriptionChanged, this,
            &ExpressionsDialog::handleDescriptionChanged);

    _graphIdx = idx;

    _pHighlighter = new ExpressionHighlighting(_pUi->lineExpression->document());

    connect(&_expressionChecker, &ExpressionChecker::resultsReady, this, &ExpressionsDialog::handleResultReady);

    populateHelpDevices();

    _pUi->tblExpressionInput->setRowCount(0);
    _pUi->tblExpressionInput->setColumnCount(2);
    _pUi->tblExpressionInput->setHorizontalHeaderLabels(QStringList() << "Data Point" << "Value");
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(_pUi->tblExpressionInput, &QTableWidget::cellChanged, this, &ExpressionsDialog::handleCellChanged);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);

    setupButtons();
    setupExpressionEditor();
    setupInsertDataPoint();

    connect(_pUi->lineExpression, &QPlainTextEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);
    _pUi->lineExpression->setPlainText(_pGraphDataModel->expression(_graphIdx));
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}

/*!
 * \brief Handle Enter in the expression editor and in the insert data point form.
 *
 * In the expression editor, Enter accepts when the expression can be applied. Shift+Enter is
 * ignored, so the expression never gets a line break. In the insert data point form, Enter
 * inserts the data point.
 * \param pWatched The object that received the event.
 * \param pEvent   The event.
 * \return true when the event was handled here.
 */
bool ExpressionsDialog::eventFilter(QObject* pWatched, QEvent* pEvent)
{
    const bool bWatched = pWatched == _pUi->lineExpression || pWatched == _pUi->widgetInsertDataPoint;
    if (!bWatched || pEvent->type() != QEvent::KeyPress)
    {
        return QDialog::eventFilter(pWatched, pEvent);
    }

    const auto* pKeyEvent = static_cast<QKeyEvent*>(pEvent); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (!isEnterKey(pKeyEvent))
    {
        return QDialog::eventFilter(pWatched, pEvent);
    }

    if (pWatched == _pUi->widgetInsertDataPoint)
    {
        insertDataPoint();
    }
    else if (!pKeyEvent->modifiers().testFlag(Qt::ShiftModifier))
    {
        handleAccept();
    }
    return true;
}

/*!
 * \brief Ignore Enter outside the expression editor and the buttons.
 *
 * The button box makes Accept the default button when the dialog is shown. Without this,
 * Enter in the example input table would apply the expression by accident. Enter on a focused
 * button still presses that button.
 * \param pEvent The key event.
 */
void ExpressionsDialog::keyPressEvent(QKeyEvent* pEvent)
{
    if (isEnterKey(pEvent))
    {
        pEvent->accept();
        return;
    }

    QDialog::keyPressEvent(pEvent);
}

/*!
 * \brief Return whether the key event is Return or Enter.
 * \param pEvent The key event.
 */
bool ExpressionsDialog::isEnterKey(const QKeyEvent* pEvent)
{
    return pEvent->key() == Qt::Key_Return || pEvent->key() == Qt::Key_Enter;
}

/*!
 * \brief Add the Accept button to the button box and set the tab order.
 */
void ExpressionsDialog::setupButtons()
{
    _pAcceptButton = _pUi->buttonBox->addButton(QStringLiteral("&Accept"), QDialogButtonBox::AcceptRole);
    _pAcceptButton->setEnabled(false);

    QPushButton* pCancelButton = _pUi->buttonBox->button(QDialogButtonBox::Cancel);

    connect(_pUi->buttonBox, &QDialogButtonBox::accepted, this, &ExpressionsDialog::handleAccept);
    connect(_pUi->buttonBox, &QDialogButtonBox::rejected, this, &ExpressionsDialog::handleCancel);

    setTabOrder(_pUi->lineExpression, _pUi->btnInsertDataPoint);
    setTabOrder(_pUi->btnInsertDataPoint, _pUi->btnInsertAtCursor);
    setTabOrder(_pUi->btnInsertAtCursor, _pUi->tblExpressionInput);
    setTabOrder(_pUi->tblExpressionInput, _pAcceptButton);
    setTabOrder(_pAcceptButton, pCancelButton);
    setTabOrder(pCancelButton, _pUi->btnInfo);
    setTabOrder(_pUi->btnInfo, _pUi->cmbHelpDevice);
}

/*!
 * \brief Size the expression editor to three lines and handle Enter in it.
 *
 * Longer expressions wrap and show a vertical scroll bar.
 */
void ExpressionsDialog::setupExpressionEditor()
{
    QPlainTextEdit* pEditor = _pUi->lineExpression;
    const int textHeight = pEditor->fontMetrics().lineSpacing() * cExpressionEditorLines;
    const int margins = static_cast<int>(2 * pEditor->document()->documentMargin()) + 2 * pEditor->frameWidth() +
                        pEditor->contentsMargins().top() + pEditor->contentsMargins().bottom();
    pEditor->setFixedHeight(textHeight + margins);

    pEditor->installEventFilter(this);
}

/*!
 * \brief Hide the insert data point form until Insert data point is checked.
 */
void ExpressionsDialog::setupInsertDataPoint()
{
    _pUi->widgetInsertDataPoint->setVisible(false);
    _pUi->btnInsertAtCursor->setEnabled(false);
    _pUi->widgetInsertDataPoint->installEventFilter(this);

    /* Errors come from the adapter; never interpret them as rich text */
    _pUi->lblInsertError->setTextFormat(Qt::PlainText);
    _pUi->lblError->setTextFormat(Qt::PlainText);

    connect(_pUi->btnInsertDataPoint, &QAbstractButton::toggled, this,
            &ExpressionsDialog::handleInsertDataPointToggled);
    connect(_pUi->btnInsertAtCursor, &QAbstractButton::clicked, this, &ExpressionsDialog::insertDataPoint);
}

/*!
 * \brief Show or hide the insert data point form.
 *
 * The form is created the first time it is shown, so the dialog does not ask the adapter to build a
 * data point when the form is never used. It starts on the device of the first data point in the
 * expression.
 * \param bChecked Whether the form is shown.
 */
void ExpressionsDialog::handleInsertDataPointToggled(bool bChecked)
{
    if (bChecked && _pInsertEditor == nullptr)
    {
        _pInsertEditor = new DataPointEditorWidget(_pSettingsModel, _pDataPointService, _pUi->insertEditorContainer);
        _pUi->insertEditorLayout->addWidget(_pInsertEditor);

        connect(_pInsertEditor, &DataPointEditorWidget::expressionChanged, this,
                &ExpressionsDialog::updateInsertButton);
        connect(_pInsertEditor, &DataPointEditorWidget::errorChanged, _pUi->lblInsertError, &QLabel::setText);
        connect(_pInsertEditor, &DataPointEditorWidget::errorChanged, this, &ExpressionsDialog::updateInsertButton);

        const QList<ExpressionReferences::Reference> references =
          ExpressionReferences::find(_pUi->lineExpression->toPlainText());
        if (!references.isEmpty() && _pSettingsModel->hasDevice(references.first().deviceId))
        {
            _pInsertEditor->setDevice(references.first().deviceId);
        }

        setTabOrder(_pUi->btnInsertDataPoint, _pInsertEditor);
        setTabOrder(_pInsertEditor, _pUi->btnInsertAtCursor);
        updateInsertButton();
    }

    _pUi->widgetInsertDataPoint->setVisible(bChecked);
}

/*!
 * \brief Allow Insert at cursor only while the form holds a built data point reference.
 */
void ExpressionsDialog::updateInsertButton()
{
    _pUi->btnInsertAtCursor->setEnabled(_pInsertEditor != nullptr && _pInsertEditor->hasValidExpression());
}

/*!
 * \brief Insert the data point reference of the form at the cursor of the expression editor.
 *
 * Replaces the selected text, if any. Does nothing while the form has no built reference.
 */
void ExpressionsDialog::insertDataPoint()
{
    if (_pInsertEditor == nullptr || !_pInsertEditor->hasValidExpression())
    {
        return;
    }

    QTextCursor cursor = _pUi->lineExpression->textCursor();
    cursor.insertText(_pInsertEditor->expression());
    _pUi->lineExpression->setTextCursor(cursor);
    _pUi->lineExpression->setFocus();
}

/*!
 * \brief Fill the data point table with one row per reference in the expression, in expression order.
 *
 * Example values are kept per reference text, so a reference that stays in the expression keeps its
 * value and duplicate references share one.
 */
void ExpressionsDialog::handleExpressionChange()
{
    /* Avoid endless signal loop, because formatting also emit textChanged */
    QString expression = _pUi->lineExpression->toPlainText();
    if (_expressionChecker.expression() == expression)
    {
        return;
    }

    _expressionChecker.setExpression(expression);

    /* Save current test values keyed by reference text, not displayed text */
    QMap<QString, QString> testValueMap;
    for (qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
    {
        QTableWidgetItem* pAddrItem = _pUi->tblExpressionInput->item(idx, 0);
        QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
        if (pAddrItem != nullptr && pValueItem != nullptr)
        {
            testValueMap.insert(pAddrItem->data(cReferenceTextRole).toString(), pValueItem->text());
        }
    }

    const QList<ExpressionReferences::Reference> references = ExpressionReferences::find(expression);

    _bUpdating = true;
    _pUi->tblExpressionInput->setRowCount(static_cast<int>(references.size()));
    _pHighlighter->setExpressionErrorPosition(-1);

    for (qint32 idx = 0; idx < references.size(); idx++)
    {
        const ExpressionReferences::Reference& reference = references[idx];

        auto* pRegItem = new QTableWidgetItem(DataPoint(reference.text, reference.deviceId).description());
        pRegItem->setFlags(pRegItem->flags() & ~Qt::ItemIsEditable);
        pRegItem->setData(cReferenceTextRole, reference.text);
        pRegItem->setData(cDeviceIdRole, reference.deviceId);
        _pUi->tblExpressionInput->setItem(idx, 0, pRegItem);

        QString testVal = testValueMap.value(reference.text, QStringLiteral("1"));
        _pUi->tblExpressionInput->setItem(idx, 1, new QTableWidgetItem(testVal));
    }

    _bUpdating = false;

    for (qint32 row = 0; row < references.size(); row++)
    {
        showDescription(row);
    }

    handleInputChange();
}

/*!
 * \brief Copy an edited example value to the rows with the same reference, then evaluate.
 * \param row    Row of the changed cell.
 * \param column Column of the changed cell.
 */
void ExpressionsDialog::handleCellChanged(int row, int column)
{
    if (_bUpdating)
    {
        return;
    }

    if (column == 1)
    {
        copyValueToDuplicateRows(row);
    }

    handleInputChange();
}

/*!
 * \brief Copy the example value of a row to the other rows with the same reference.
 * \param row Row whose value changed.
 */
void ExpressionsDialog::copyValueToDuplicateRows(qint32 row)
{
    QTableWidget* pTable = _pUi->tblExpressionInput;
    const QTableWidgetItem* pChangedItem = pTable->item(row, 1);
    if (pChangedItem == nullptr)
    {
        return;
    }

    const QString referenceText = rowReferenceText(row);
    const QSignalBlocker blocker(pTable);
    for (qint32 idx = 0; idx < pTable->rowCount(); idx++)
    {
        QTableWidgetItem* pValueItem = pTable->item(idx, 1);
        if (idx != row && pValueItem != nullptr && rowReferenceText(idx) == referenceText)
        {
            pValueItem->setText(pChangedItem->text());
        }
    }
}

/*!
 * \brief Evaluate the expression with the example values in the table.
 *
 * The evaluation takes one value per distinct data point, in the order the parser lists them; each
 * value comes from the first row with that reference.
 */
void ExpressionsDialog::handleInputChange()
{
    if (_bUpdating)
    {
        return;
    }

    const auto lightRed = QColor(255, 0, 0, 127);
    const auto white = QColorConstants::White;

    QTableWidget* pTable = _pUi->tblExpressionInput;

    /* Avoid recursive signal/slots calling */
    const QSignalBlocker blocker(pTable);

    QMap<QString, ResultDouble> valueMap;
    for (qint32 idx = 0; idx < pTable->rowCount(); idx++)
    {
        QTableWidgetItem* pValueItem = pTable->item(idx, 1);
        if (pValueItem == nullptr)
        {
            continue;
        }

        bool bOk = false;
        double value = ExpressionValue::parse(pValueItem->text(), &bOk);

        pValueItem->setBackground(bOk ? white : lightRed);
        pValueItem->setToolTip(bOk ? QString() : QStringLiteral("Not a valid number"));

        const QString referenceText = rowReferenceText(idx);
        if (!valueMap.contains(referenceText))
        {
            valueMap.insert(referenceText, ResultDouble(value, bOk ? State::Good : State::Invalid));
        }
    }

    ResultDoubleList results;
    const QStringList addresses = _expressionChecker.addresses();
    for (const QString& address : addresses)
    {
        results.append(valueMap.value(address, ResultDouble(0, State::Invalid)));
    }

    _expressionChecker.checkWithValues(results);
}

/*!
 * \brief Return the data point reference of a table row.
 * \param row Row index into the data point table.
 */
QString ExpressionsDialog::rowReferenceText(qint32 row) const
{
    const QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
    if (pItem == nullptr)
    {
        return QString();
    }

    return pItem->data(cReferenceTextRole).toString();
}

void ExpressionsDialog::handleCancel()
{
    done(QDialog::Rejected);
}

/*!
 * \brief Apply the expression to the signal and close the dialog.
 *
 * Does nothing while the expression has a syntax error or refers to an unknown device.
 */
void ExpressionsDialog::handleAccept()
{
    if (!_bCanAccept)
    {
        return;
    }

    _pGraphDataModel->setExpression(_graphIdx, _pUi->lineExpression->toPlainText().trimmed());

    done(QDialog::Accepted);
}

void ExpressionsDialog::handleExpressionHelpResult(const QString& helpText)
{
    const QString fullHelpHtml =
      QStringLiteral("<html><head/><body>") + genericExpressionInfoHtml() + helpText + QStringLiteral("</body></html>");
    _pUi->lblInfo->setText(fullHelpHtml);
}

/*!
 * \brief Fill the help device selector and request help text for the initial selection.
 *
 * Each entry represents a configured device, labeled with its name, protocol and id.
 * The selector is hidden when fewer than two devices are available.
 */
void ExpressionsDialog::populateHelpDevices()
{
    if (_pAdapterHub == nullptr || _pSettingsModel == nullptr)
    {
        _pUi->widgetHelpDevice->setVisible(false);
        return;
    }

    _pUi->cmbHelpDevice->setLabelStyle(DeviceComboBox::LabelStyle::NameProtocolId);
    _pUi->cmbHelpDevice->setSettingsModel(_pSettingsModel);
    _pUi->widgetHelpDevice->setVisible(_pUi->cmbHelpDevice->count() > 1);

    connect(_pUi->cmbHelpDevice, &DeviceComboBox::deviceChanged, this, &ExpressionsDialog::onHelpDeviceChanged);

    if (_pUi->cmbHelpDevice->count() > 0)
    {
        onHelpDeviceChanged(_pUi->cmbHelpDevice->selectedDeviceId());
    }
}

/*!
 * \brief Request expression help text from the given adapter.
 *
 * Any pending help connection to a previously selected adapter is dropped so a
 * late response cannot overwrite the newly selected adapter's help text.
 * A no-op if the given adapter is already the one being shown, since multiple
 * devices can share the same adapter and switching between them should not
 * flicker the help text or trigger a redundant request.
 * \param adapterId Identifier of the adapter to request help from.
 */
void ExpressionsDialog::requestHelpForAdapter(const QString& adapterId)
{
    if (_pHelpManager != nullptr && _pHelpManager->adapterId() == adapterId)
    {
        return;
    }

    if (_pHelpManager != nullptr)
    {
        QObject::disconnect(_pHelpManager, &AdapterManager::expressionHelpResult, this,
                            &ExpressionsDialog::handleExpressionHelpResult);
    }

    _pUi->lblInfo->clear();

    _pHelpManager = _pAdapterHub->adapterManager(adapterId);
    if (_pHelpManager == nullptr)
    {
        return;
    }

    connect(_pHelpManager, &AdapterManager::expressionHelpResult, this, &ExpressionsDialog::handleExpressionHelpResult,
            Qt::SingleShotConnection);
    _pHelpManager->requestExpressionHelp();
}

/*!
 * \brief Show the expression help of the newly selected device's adapter.
 * \param deviceId The selected device.
 */
void ExpressionsDialog::onHelpDeviceChanged(deviceId_t deviceId)
{
    if (!_pSettingsModel->hasDevice(deviceId))
    {
        requestHelpForAdapter(QString());
        return;
    }

    requestHelpForAdapter(_pSettingsModel->adapterIdForDevice(deviceId));
}

/*!
 * \brief Show the adapter's description of the data point in a table row, when it is known.
 *
 * A row with an unknown device shows "Invalid device". A data point the adapter rejects shows its
 * reference in red, with the adapter's error as tooltip. A row whose description is not available
 * (pending, failed or adapter not running) keeps the parser description.
 * \param row Row index into the data point table.
 */
void ExpressionsDialog::showDescription(qint32 row)
{
    QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
    if (pItem == nullptr)
    {
        return;
    }

    /* The description column does not change the evaluation, so don't evaluate again */
    const QSignalBlocker blocker(_pUi->tblExpressionInput);

    pItem->setData(cInvalidErrorRole, QString());
    pItem->setData(Qt::ForegroundRole, QVariant());
    pItem->setToolTip(QString());

    const auto deviceId = pItem->data(cDeviceIdRole).value<deviceId_t>();
    if (!_pSettingsModel->hasDevice(deviceId))
    {
        pItem->setText(QStringLiteral("Invalid device"));
        return;
    }

    const QString referenceText = pItem->data(cReferenceTextRole).toString();
    const DataPointDescription description =
      _pDescriptionCache->lookup({ 0, referenceText.size(), referenceText, deviceId });
    if (description.state == DataPointDescription::State::Valid && !description.description.isEmpty())
    {
        pItem->setText(description.description);
    }
    else if (description.state == DataPointDescription::State::Invalid)
    {
        const QString error =
          description.error.isEmpty() ? QStringLiteral("Not a valid data point") : description.error;
        pItem->setText(referenceText);
        pItem->setForeground(QColorConstants::Red);
        pItem->setToolTip(Util::plainTextToolTip(error));
        pItem->setData(cInvalidErrorRole, error);
    }
}

/*!
 * \brief Return the adapter's error for the first data point it rejects, or an empty string.
 */
QString ExpressionsDialog::firstInvalidReferenceError() const
{
    for (qint32 row = 0; row < _pUi->tblExpressionInput->rowCount(); row++)
    {
        const QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
        if (pItem != nullptr && !pItem->data(cInvalidErrorRole).toString().isEmpty())
        {
            return pItem->data(cInvalidErrorRole).toString();
        }
    }

    return QString();
}

/*!
 * \brief Show a description that arrived for a data point in the table.
 * \param referenceText The data point reference whose description changed.
 */
void ExpressionsDialog::handleDescriptionChanged(const QString& referenceText)
{
    for (qint32 row = 0; row < _pUi->tblExpressionInput->rowCount(); row++)
    {
        if (rowReferenceText(row) == referenceText)
        {
            showDescription(row);
        }
    }

    updateAcceptState();
}

/*!
 * \brief Show the evaluation result and the error state.
 * \param valid Whether the expression evaluated with the example values.
 */
void ExpressionsDialog::handleResultReady(bool valid)
{
    _pHighlighter->setExpressionErrorPosition(_expressionChecker.errorPos());
    _pHighlighter->rehighlight();

    if (valid)
    {
        _pUi->lblOut->setText(QString("%1").arg(_expressionChecker.result()));
    }
    else
    {
        _pUi->lblOut->setText(QStringLiteral("-"));
    }

    updateAcceptState();
}

/*!
 * \brief Show the error and allow Accept only when the expression can be applied.
 *
 * An error that depends on the example values (such as a division by zero) is shown but does not
 * block Accept; a syntax error, a reference to an unknown device or a data point the adapter
 * rejects does.
 */
void ExpressionsDialog::updateAcceptState()
{
    const QString errorStyle = QStringLiteral("background-color: rgba(255,0,0,50%);");

    QString backgroundStyle = QStringLiteral("background-color: rgba(0,0,0,0%);");
    QString strError;
    if (!_expressionChecker.isValid())
    {
        strError = _expressionChecker.strError();
        backgroundStyle = errorStyle;
    }

    const bool bDevicesKnown = _expressionChecker.checkForDevices(_pSettingsModel->deviceList());
    if (!bDevicesKnown && !_expressionChecker.syntaxError())
    {
        strError = QStringLiteral("Unknown device in expression");
        backgroundStyle = errorStyle;
    }

    /* A rejected data point blocks Accept, so it is shown over an error of the example values */
    const QString invalidReferenceError = firstInvalidReferenceError();
    if (!invalidReferenceError.isEmpty() && bDevicesKnown && !_expressionChecker.syntaxError())
    {
        strError = QStringLiteral("Invalid data point: %1").arg(invalidReferenceError);
        backgroundStyle = errorStyle;
    }

    _bCanAccept = !_expressionChecker.syntaxError() && bDevicesKnown && invalidReferenceError.isEmpty();
    _pAcceptButton->setEnabled(_bCanAccept);

    _pUi->lblError->setText(strError);
    _pUi->lblError->setStyleSheet(backgroundStyle);
}
