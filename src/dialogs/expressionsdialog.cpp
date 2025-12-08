#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "dialogs/expressionhighlighting.h"
#include "models/graphdatamodel.h"

using State = ResultState::State;

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, qint32 idx, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _pGraphDataModel(pGraphDataModel),
    _bUpdating(false)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    _pHighlighter = new ExpressionHighlighting(_pUi->lineExpression->document());

    connect(&_expressionChecker, &ExpressionChecker::resultsReady, this, &ExpressionsDialog::handleResultReady);

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

        /* Save current test values */
        QMap<QString, QString> testValueMap;
        for(qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QString descr = _pUi->tblExpressionInput->item(idx, 0)->text();
            QString value = _pUi->tblExpressionInput->item(idx, 1)->text();

            testValueMap.insert(descr, value);
        }

        QStringList descriptions;
        _expressionChecker.descriptions(descriptions);

        _bUpdating = true;
        _pUi->tblExpressionInput->setRowCount(descriptions.size());
        _pHighlighter->setExpressionErrorPosition(-1);

        for(qint32 idx = 0; idx < descriptions.size(); idx++)
        {
            QString regDescr = descriptions[idx];
            QTableWidgetItem *newRegisterDescr = new QTableWidgetItem(regDescr);
            newRegisterDescr->setFlags(newRegisterDescr->flags() & ~Qt::ItemIsEditable);
            _pUi->tblExpressionInput->setItem(idx, 0, newRegisterDescr);

            QString testVal = testValueMap.contains(regDescr) ? testValueMap[regDescr]: "0";
            QTableWidgetItem *newRegisterValue = new QTableWidgetItem(testVal);
            _pUi->tblExpressionInput->setItem(idx, 1, newRegisterValue);
        }

        _bUpdating = false;

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
        for(qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
            QString valueStr = pValueItem->text();
            bool bOk = false;
            double value = valueStr.toDouble(&bOk);
            results.append(ResultDouble(value, bOk ? State::SUCCESS: State::INVALID));

            /* Avoid recursive signal/slots calling */
            _pUi->tblExpressionInput->blockSignals(true);

            pValueItem->setBackground(bOk ? white: lightRed);
            pValueItem->setToolTip(bOk ? QString(): QStringLiteral("Not a valid number"));

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
        numOutput = QString("%0").arg(_expressionChecker.result());
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
