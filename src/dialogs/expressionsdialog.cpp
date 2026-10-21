#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "dialogs/expressionhighlighting.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"
#include "util/expressionvalue.h"

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
                          "Both a decimal point and a comma can be used as the decimal separator. "
                          "The first one encountered is used as the decimal separator for the whole expression.</p>");
}

} // namespace

ExpressionsDialog::ExpressionsDialog(GraphDataModel* pGraphDataModel,
                                     GraphIdx idx,
                                     AdapterHub* pAdapterHub,
                                     SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     QWidget* parent)
    : QDialog(parent),
      _pUi(new Ui::ExpressionsDialog),
      _pGraphDataModel(pGraphDataModel),
      _pAdapterHub(pAdapterHub),
      _pSettingsModel(pSettingsModel),
      _pDataPointService(pDataPointService),
      _bUpdating(false)
{
    _pUi->setupUi(this);

    connect(_pDataPointService, &DataPointService::dataPointDescribed, this,
            &ExpressionsDialog::handleDataPointDescribed);
    connect(_pDataPointService, &DataPointService::requestFailed, this, &ExpressionsDialog::handleDescribeFailed);

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
    connect(_pUi->btnCancel, &QPushButton::clicked, this, &ExpressionsDialog::handleCancel);
    connect(_pUi->btnAccept, &QPushButton::clicked, this, &ExpressionsDialog::handleAccept);
    _pUi->btnAccept->setEnabled(false);

    connect(_pUi->lineExpression, &QPlainTextEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);
    _pUi->lineExpression->setPlainText(_pGraphDataModel->expression(_graphIdx));
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
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

        requestDescriptions(addresses, _expressionChecker.deviceIds());

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

    const QList<deviceId_t> deviceIds = _pSettingsModel->deviceList();
    for (deviceId_t deviceId : deviceIds)
    {
        Device* pDevice = _pSettingsModel->deviceSettings(deviceId);
        const QString adapterId = pDevice->adapterId();

        const QString protocolLabel = _pSettingsModel->adapterDisplayName(adapterId);
        const QString label = QStringLiteral("%1 (%2) [%3]").arg(pDevice->name(), protocolLabel).arg(deviceId);
        _pUi->cmbHelpDevice->addItem(label, adapterId);
    }
    _pUi->widgetHelpDevice->setVisible(deviceIds.size() > 1);

    /* Connect after populating to avoid spurious slot calls during addItem */
    connect(_pUi->cmbHelpDevice, &QComboBox::currentIndexChanged, this, &ExpressionsDialog::onHelpDeviceChanged);

    if (!deviceIds.isEmpty())
    {
        requestHelpForAdapter(_pUi->cmbHelpDevice->currentData().toString());
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
 * \param index Index of the newly selected combo box entry (unused).
 */
void ExpressionsDialog::onHelpDeviceChanged(int index)
{
    Q_UNUSED(index);
    requestHelpForAdapter(_pUi->cmbHelpDevice->currentData().toString());
}

/*!
 * \brief Ask the adapters for a description of every data point in the table.
 *
 * Replies to requests for a previous expression are ignored. A row whose adapter is unavailable
 * keeps the parser description; a row with an unknown device shows "Invalid device".
 * \param addresses Data point reference of each table row.
 * \param deviceIds Device of each table row.
 */
void ExpressionsDialog::requestDescriptions(const QStringList& addresses, const QList<deviceId_t>& deviceIds)
{
    _describeRowByTicket.clear();

    for (qint32 row = 0; row < addresses.size() && row < deviceIds.size(); row++)
    {
        if (!_pSettingsModel->hasDevice(deviceIds[row]))
        {
            setDescribeRowText(row, QStringLiteral("Invalid device"));
            continue;
        }

        if (_pDataPointService->isDeviceUsable(deviceIds[row]))
        {
            const quint64 ticket = _pDataPointService->describeDataPoint(deviceIds[row], addresses[row]);
            _describeRowByTicket.insert(ticket, row);
        }
    }
}

/*!
 * \brief Set the description text of a pending-describe row, if the row still has an item.
 * \param row  Row index into the describe table.
 * \param text Text to set; ignored when empty.
 */
void ExpressionsDialog::setDescribeRowText(qint32 row, const QString& text)
{
    QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(row, 0);
    if (pItem != nullptr && !text.isEmpty())
    {
        pItem->setText(text);
    }
}

/*!
 * \brief Show the adapter's description of a data point in its table row.
 * \param ticket Ticket of the describe request; replies for other requests are ignored.
 * \param result The describeDataPoint result.
 */
void ExpressionsDialog::handleDataPointDescribed(quint64 ticket, const QJsonObject& result)
{
    if (!_describeRowByTicket.contains(ticket))
    {
        return;
    }

    setDescribeRowText(_describeRowByTicket.take(ticket), result["description"].toString());
}

/*!
 * \brief Keep the parser description of a row whose describe request failed.
 * \param ticket Ticket of the failed request.
 * \param error  Why the request failed (unused).
 */
void ExpressionsDialog::handleDescribeFailed(quint64 ticket, const QString& error)
{
    Q_UNUSED(error);
    _describeRowByTicket.remove(ticket);
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
    _pUi->btnAccept->setEnabled(_bCanAccept);

    _pUi->lblOut->setText(numOutput);

    _pUi->lblError->setText(strError);
    _pUi->lblError->setStyleSheet(backgroundStyle);
}
