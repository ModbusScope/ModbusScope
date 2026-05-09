#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "ProtocolAdapter/adaptermanager.h"
#include "dialogs/expressionhighlighting.h"
#include "models/graphdatamodel.h"

using State = ResultState::State;

ExpressionsDialog::ExpressionsDialog(GraphDataModel* pGraphDataModel,
                                     qint32 idx,
                                     AdapterManager* pAdapterManager,
                                     QWidget* parent)
    : QDialog(parent),
      _pUi(new Ui::ExpressionsDialog),
      _pGraphDataModel(pGraphDataModel),
      _pAdapterManager(pAdapterManager),
      _bUpdating(false)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    _pHighlighter = new ExpressionHighlighting(_pUi->lineExpression->document());

    connect(&_expressionChecker, &ExpressionChecker::resultsReady, this, &ExpressionsDialog::handleResultReady);

    if (_pAdapterManager != nullptr)
    {
        connect(_pAdapterManager, &AdapterManager::expressionHelpResult, this,
                &ExpressionsDialog::handleExpressionHelpResult);
        _pAdapterManager->requestExpressionHelp();
    }

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
        _nextDescribeRow = 0;
        if (_pAdapterManager != nullptr)
        {
            QObject::disconnect(_pAdapterManager, &AdapterManager::describeDataPointResult, this,
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

void ExpressionsDialog::startNextDescribe()
{
    if (_pAdapterManager != nullptr && !_pAdapterManager->isAdapterIdle() &&
        _nextDescribeRow < _pendingDescribeAddresses.size())
    {
        connect(_pAdapterManager, &AdapterManager::describeDataPointResult, this,
                &ExpressionsDialog::handleDescribeDataPointResult, Qt::SingleShotConnection);
        _pAdapterManager->describeDataPoint(_pendingDescribeAddresses[_nextDescribeRow]);
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
