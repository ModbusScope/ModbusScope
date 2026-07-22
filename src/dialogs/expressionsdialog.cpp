#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "dialogs/expressionhighlighting.h"
#include "models/adapterdata.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

using State = ResultState::State;

ExpressionsDialog::ExpressionsDialog(GraphDataModel* pGraphDataModel,
                                     GraphIdx idx,
                                     AdapterHub* pAdapterHub,
                                     SettingsModel* pSettingsModel,
                                     QWidget* parent)
    : QDialog(parent),
      _pUi(new Ui::ExpressionsDialog),
      _pGraphDataModel(pGraphDataModel),
      _pAdapterHub(pAdapterHub),
      _pSettingsModel(pSettingsModel),
      _bUpdating(false)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    _pHighlighter = new ExpressionHighlighting(_pUi->lineExpression->document());

    connect(&_expressionChecker, &ExpressionChecker::resultsReady, this, &ExpressionsDialog::handleResultReady);

    populateHelpAdapters();

    _pUi->tblExpressionInput->setRowCount(0);
    _pUi->tblExpressionInput->setColumnCount(2);
    _pUi->tblExpressionInput->setHorizontalHeaderLabels(QStringList() << "Register" << "Value");
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(_pUi->tblExpressionInput, &QTableWidget::cellChanged, this, &ExpressionsDialog::handleInputChange);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);
    connect(_pUi->btnCancel, &QPushButton::clicked, this, &ExpressionsDialog::handleCancel);
    connect(_pUi->btnAccept, &QPushButton::clicked, this, &ExpressionsDialog::handleAccept);

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

            QString testVal = testValueMap.value(addresses[idx], QStringLiteral("0"));
            _pUi->tblExpressionInput->setItem(idx, 1, new QTableWidgetItem(testVal));
        }

        _bUpdating = false;

        _pendingDescribeAddresses = addresses;
        _pendingDescribeDeviceIds = _expressionChecker.deviceIds();
        _nextDescribeRow = 0;
        if (_pDescribeManager != nullptr)
        {
            QObject::disconnect(_pDescribeManager, &AdapterManager::describeDataPointResult, this,
                                &ExpressionsDialog::handleDescribeDataPointResult);
        }
        startNextDescribe();

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
            double value = valueStr.toDouble(&bOk);
            results.append(ResultDouble(value, bOk ? State::SUCCESS : State::INVALID));

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

void ExpressionsDialog::handleAccept()
{
    _pGraphDataModel->setExpression(_graphIdx, _pUi->lineExpression->toPlainText().trimmed());

    done(QDialog::Accepted);
}

void ExpressionsDialog::handleExpressionHelpResult(const QString& helpText)
{
    _pUi->lblInfo->setText(helpText);
}

/*!
 * \brief Fill the help adapter selector and request help text for the initial selection.
 *
 * The selector is hidden when fewer than two adapters are available.
 */
void ExpressionsDialog::populateHelpAdapters()
{
    if (_pAdapterHub == nullptr)
    {
        _pUi->widgetHelpAdapter->setVisible(false);
        return;
    }

    const QStringList adapterIds = _pAdapterHub->adapterIds();
    for (const QString& adapterId : adapterIds)
    {
        QString label;
        if (_pSettingsModel != nullptr)
        {
            label = _pSettingsModel->adapterData(adapterId)->name();
        }
        if (label.isEmpty())
        {
            label = adapterId;
        }
        _pUi->cmbHelpAdapter->addItem(label, adapterId);
    }
    _pUi->widgetHelpAdapter->setVisible(adapterIds.size() > 1);

    /* Connect after populating to avoid spurious slot calls during addItem */
    connect(_pUi->cmbHelpAdapter, &QComboBox::currentIndexChanged, this, &ExpressionsDialog::onHelpAdapterChanged);

    if (!adapterIds.isEmpty())
    {
        requestHelpForAdapter(_pUi->cmbHelpAdapter->currentData().toString());
    }
}

/*!
 * \brief Request expression help text from the given adapter.
 *
 * Any pending help connection to a previously selected adapter is dropped so a
 * late response cannot overwrite the newly selected adapter's help text.
 * \param adapterId Identifier of the adapter to request help from.
 */
void ExpressionsDialog::requestHelpForAdapter(const QString& adapterId)
{
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

    connect(_pHelpManager, &AdapterManager::expressionHelpResult, this,
            &ExpressionsDialog::handleExpressionHelpResult, Qt::SingleShotConnection);
    _pHelpManager->requestExpressionHelp();
}

/*!
 * \brief Show the expression help of the newly selected adapter.
 * \param index Index of the newly selected combo box entry (unused).
 */
void ExpressionsDialog::onHelpAdapterChanged(int index)
{
    Q_UNUSED(index);
    requestHelpForAdapter(_pUi->cmbHelpAdapter->currentData().toString());
}

/*!
 * \brief Resolve the adapter manager responsible for the data point at the given row.
 *
 * The row's device ID is mapped to its owning adapter via the settings model.
 * \param row Row index into the pending describe lists.
 * \return Pointer to the manager, or nullptr when it is unknown or not running.
 */
AdapterManager* ExpressionsDialog::managerForDescribeRow(qint32 row) const
{
    if (_pAdapterHub == nullptr || _pSettingsModel == nullptr || row >= _pendingDescribeDeviceIds.size())
    {
        return nullptr;
    }

    const QString adapterId = _pSettingsModel->adapterIdForDevice(_pendingDescribeDeviceIds[row]);
    AdapterManager* pManager = _pAdapterHub->adapterManager(adapterId);
    if (pManager == nullptr || pManager->isAdapterIdle())
    {
        return nullptr;
    }

    return pManager;
}

void ExpressionsDialog::startNextDescribe()
{
    _pDescribeManager = nullptr;
    while (_nextDescribeRow < _pendingDescribeAddresses.size() && _pDescribeManager == nullptr)
    {
        AdapterManager* pManager = managerForDescribeRow(_nextDescribeRow);
        if (pManager != nullptr)
        {
            _pDescribeManager = pManager;
            connect(pManager, &AdapterManager::describeDataPointResult, this,
                    &ExpressionsDialog::handleDescribeDataPointResult, Qt::SingleShotConnection);
            pManager->describeDataPoint(_pendingDescribeAddresses[_nextDescribeRow]);
        }
        else
        {
            /* Adapter unavailable for this row: keep the parser description and try the next row */
            _nextDescribeRow++;
        }
    }
}

void ExpressionsDialog::handleDescribeDataPointResult(const QJsonObject& result)
{
    if (_nextDescribeRow >= _pendingDescribeAddresses.size())
    {
        return;
    }

    QString description = result["description"].toString();
    QTableWidgetItem* pItem = _pUi->tblExpressionInput->item(_nextDescribeRow, 0);
    if (pItem != nullptr && !description.isEmpty())
    {
        pItem->setText(description);
    }

    _nextDescribeRow++;
    startNextDescribe();
}

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
    _pUi->lblOut->setText(numOutput);

    _pUi->lblError->setText(strError);
    _pUi->lblError->setStyleSheet(backgroundStyle);
}
