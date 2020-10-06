#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "graphdatamodel.h"

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, qint32 idx, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _expressionParser("VAL"),
    _pGraphDataModel(pGraphDataModel)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);
    connect(_pUi->btnCancel, &QPushButton::clicked, this, &ExpressionsDialog::handleCancel);
    connect(_pUi->btnAccept, &QPushButton::clicked, this, &ExpressionsDialog::handleAccept);

    _pUi->lineExpression->setText(_pGraphDataModel->expression(_graphIdx));
    connect(_pUi->lineExpression, &QLineEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);

    handleExpressionChange();

    connect(_pUi->lineIn0, &QLineEdit::textChanged, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn1, &QLineEdit::textChanged, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn2, &QLineEdit::textChanged, this, &ExpressionsDialog::handleInputChange);
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}

void ExpressionsDialog::handleExpressionChange()
{
    _expressionParser.setExpression(_pUi->lineExpression->text());

    handleInputChange();
}

void ExpressionsDialog::handleInputChange()
{
    QString numOutput;
    QString strTooltip;

    evaluateValue(_pUi->lineIn0->text(), numOutput, strTooltip);
    _pUi->lblOut0->setText(numOutput);
    _pUi->lblOut0->setToolTip(strTooltip);

    evaluateValue(_pUi->lineIn1->text(), numOutput, strTooltip);
    _pUi->lblOut1->setText(numOutput);
    _pUi->lblOut1->setToolTip(strTooltip);

    evaluateValue(_pUi->lineIn2->text(), numOutput, strTooltip);
    _pUi->lblOut2->setText(numOutput);
    _pUi->lblOut2->setToolTip(strTooltip);
}

void ExpressionsDialog::handleCancel()
{
    done(QDialog::Rejected);
}

void ExpressionsDialog::handleAccept()
{
    _pGraphDataModel->setExpression(_graphIdx, _pUi->lineExpression->text());

    done(QDialog::Accepted);
}

void ExpressionsDialog::evaluateValue(QString strInput, QString &numOutput, QString &strTooltip)
{
    if (!strInput.isEmpty())
    {
        bool bOk;
        int value = strInput.toInt(&bOk);
        if (bOk)
        {
            if (_expressionParser.evaluate(value))
            {
                numOutput = QString("%0").arg(_expressionParser.result());
                strTooltip = QString();
            }
            else
            {
                numOutput = QStringLiteral("-");
                strTooltip = _expressionParser.msg();
            }
        }
        else
        {
            numOutput = QStringLiteral("-");
            strTooltip = QStringLiteral("Not a valid number");
        }
    }
    else
    {
        numOutput = QString();
        strTooltip = QString();
    }
}
