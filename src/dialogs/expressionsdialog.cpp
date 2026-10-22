#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "customwidgets/devicecombobox.h"
#include "dialogs/expressionhighlighting.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "util/expressionvalue.h"

#include <QKeyEvent>
#include <QPushButton>

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
                                     DataPointDescriptionCache* pDescriptionCache,
                                     QWidget* parent)
    : QDialog(parent),
      _pUi(new Ui::ExpressionsDialog),
      _pGraphDataModel(pGraphDataModel),
      _pAdapterHub(pAdapterHub),
      _pSettingsModel(pSettingsModel),
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

    connect(_pUi->tblExpressionInput, &QTableWidget::cellChanged, this, &ExpressionsDialog::handleInputChange);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);

    setupButtons();
    setupExpressionEditor();

    connect(_pUi->lineExpression, &QPlainTextEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);
    _pUi->lineExpression->setPlainText(_pGraphDataModel->expression(_graphIdx));
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}

/*!
 * \brief Accept the expression with Enter in the expression editor.
 *
 * Enter accepts when the expression can be applied. Shift+Enter is ignored, so the
 * expression never gets a line break.
 * \param pWatched The object that received the event.
 * \param pEvent   The event.
 * \return true when the event was handled here.
 */
bool ExpressionsDialog::eventFilter(QObject* pWatched, QEvent* pEvent)
{
    if (pWatched != _pUi->lineExpression || pEvent->type() != QEvent::KeyPress)
    {
        return QDialog::eventFilter(pWatched, pEvent);
    }

    const auto* pKeyEvent = static_cast<QKeyEvent*>(pEvent); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (!isEnterKey(pKeyEvent))
    {
        return QDialog::eventFilter(pWatched, pEvent);
    }

    if (!pKeyEvent->modifiers().testFlag(Qt::ShiftModifier))
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

    setTabOrder(_pUi->lineExpression, _pUi->tblExpressionInput);
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

void ExpressionsDialog::handleExpressionChange()
{
    /* Avoid endless signal loop, because formatting also emit textChanged */
    QString expression = _pUi->lineExpression->toPlainText();
    if (_expressionChecker.expression() != expression)
    {
        _expressionChecker.setExpression(expression);

        /* Save current test values keyed by address (Qt::UserRole), not displayed text */
        QMap<QString, QString> testValueMap;
        for (qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QTableWidgetItem* pAddrItem = _pUi->tblExpressionInput->item(idx, 0);
            QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
            if (pAddrItem != nullptr && pValueItem != nullptr)
            {
                testValueMap.insert(pAddrItem->data(Qt::UserRole).toString(), pValueItem->text());
            }
        }

        QStringList descriptions = _expressionChecker.descriptions();
        QStringList addresses = _expressionChecker.addresses();

        _bUpdating = true;
        _pUi->tblExpressionInput->setRowCount(descriptions.size());
        _pHighlighter->setExpressionErrorPosition(-1);

        for (qint32 idx = 0; idx < descriptions.size(); idx++)
        {
            QTableWidgetItem* pRegItem = new QTableWidgetItem(descriptions[idx]);
            pRegItem->setFlags(pRegItem->flags() & ~Qt::ItemIsEditable);
            pRegItem->setData(Qt::UserRole, addresses[idx]);
            _pUi->tblExpressionInput->setItem(idx, 0, pRegItem);

            QString testVal = testValueMap.value(addresses[idx], QStringLiteral("1"));
            _pUi->tblExpressionInput->setItem(idx, 1, new QTableWidgetItem(testVal));
        }

        _bUpdating = false;

        const QList<deviceId_t> deviceIds = _expressionChecker.deviceIds();
        for (qint32 row = 0; row < addresses.size() && row < deviceIds.size(); row++)
        {
            showDescription(row, deviceIds[row]);
        }

        handleInputChange();
    }
}

void ExpressionsDialog::handleInputChange()
{
    if (!_bUpdating)
    {
        const auto lightRed = QColor(255, 0, 0, 127);
        const auto white = QColorConstants::White;

        ResultDoubleList results;
        for (qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
            QString valueStr = pValueItem->text();
            bool bOk = false;
            double value = ExpressionValue::parse(valueStr, &bOk);
            results.append(ResultDouble(value, bOk ? State::Good : State::Invalid));

            /* Avoid recursive signal/slots calling */
            _pUi->tblExpressionInput->blockSignals(true);

            pValueItem->setBackground(bOk ? white : lightRed);
            pValueItem->setToolTip(bOk ? QString() : QStringLiteral("Not a valid number"));

            _pUi->tblExpressionInput->blockSignals(false);
        }

        _expressionChecker.checkWithValues(results);
    }
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
 * A row with an unknown device shows "Invalid device". A row whose description is not available
 * (pending, invalid, failed or adapter not running) keeps the parser description.
 * \param row      Row index into the data point table.
 * \param deviceId Device of the data point in that row.
 */
void ExpressionsDialog::showDescription(qint32 row, deviceId_t deviceId)
{
    QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
    if (pItem == nullptr)
    {
        return;
    }

    if (!_pSettingsModel->hasDevice(deviceId))
    {
        pItem->setText(QStringLiteral("Invalid device"));
        return;
    }

    const QString referenceText = pItem->data(Qt::UserRole).toString();
    const DataPointDescription description =
      _pDescriptionCache->lookup({ 0, referenceText.size(), referenceText, deviceId });
    if (description.state == DataPointDescription::State::Valid && !description.description.isEmpty())
    {
        pItem->setText(description.description);
    }
}

/*!
 * \brief Show a description that arrived for a data point in the table.
 * \param referenceText The data point reference whose description changed.
 */
void ExpressionsDialog::handleDescriptionChanged(const QString& referenceText)
{
    const QList<deviceId_t> deviceIds = _expressionChecker.deviceIds();
    for (qint32 row = 0; row < _pUi->tblExpressionInput->rowCount() && row < deviceIds.size(); row++)
    {
        const QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
        if (pItem != nullptr && pItem->data(Qt::UserRole).toString() == referenceText)
        {
            showDescription(row, deviceIds[row]);
        }
    }
}

/*!
 * \brief Show the evaluation result and allow Accept only when the expression can be applied.
 *
 * An error that depends on the example values (such as a division by zero) is shown but does not
 * block Accept; a syntax error or a reference to an unknown device does.
 * \param valid Whether the expression evaluated with the example values.
 */
void ExpressionsDialog::handleResultReady(bool valid)
{
    _pHighlighter->setExpressionErrorPosition(_expressionChecker.errorPos());
    _pHighlighter->rehighlight();

    QString numOutput;
    QString backgroundStyle;
    QString strError;
    if (valid)
    {
        strError = "";
        numOutput = QString("%1").arg(_expressionChecker.result());
        backgroundStyle = "background-color: rgba(0,0,0,0%);";
    }
    else
    {
        strError = _expressionChecker.strError();
        numOutput = QStringLiteral("-");
        backgroundStyle = "background-color: rgba(255,0,0,50%);";
    }

    const bool bDevicesKnown = _expressionChecker.checkForDevices(_pSettingsModel->deviceList());
    if (!bDevicesKnown && !_expressionChecker.syntaxError())
    {
        strError = QStringLiteral("Unknown device in expression");
        backgroundStyle = "background-color: rgba(255,0,0,50%);";
    }

    _bCanAccept = !_expressionChecker.syntaxError() && bDevicesKnown;
    _pAcceptButton->setEnabled(_bCanAccept);

    _pUi->lblOut->setText(numOutput);

    _pUi->lblError->setText(strError);
    _pUi->lblError->setStyleSheet(backgroundStyle);
}
