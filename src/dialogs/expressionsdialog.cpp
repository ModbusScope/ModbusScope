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

    _pUi->lineExpression->setText(_expressionParser.expression());
    connect(_pUi->lineExpression, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleExpressionChange);

    connect(_pUi->lineIn0, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn1, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn2, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);

    _pUi->lineExpression->setText(_pGraphDataModel->expression(_graphIdx));
    handleExpressionChange();
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
    _pUi->lblOut0->setText(evaluateValue(_pUi->lineIn0->text()));
    _pUi->lblOut1->setText(evaluateValue(_pUi->lineIn1->text()));
    _pUi->lblOut2->setText(evaluateValue(_pUi->lineIn2->text()));
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

QString ExpressionsDialog::evaluateValue(QString strInput)
{
    QString output;

    if (!strInput.isEmpty())
    {
        bool bOk;
        int value = strInput.toInt(&bOk);
        if (bOk)
        {
            if (_expressionParser.evaluate(value))
            {
                output = QString("%0").arg(_expressionParser.result());
            }
            else
            {
                output = _expressionParser.msg();
            }
        }
        else
        {
            output = QStringLiteral("Not a valid number");
        }
    }

    return output;
}
