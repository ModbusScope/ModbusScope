#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "graphdatamodel.h"

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _expressionParser("VAL"),
    _pGraphDataModel(pGraphDataModel)
{
    _pUi->setupUi(this);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);

    /* Fill combo box */
    if (_pGraphDataModel->size() > 0)
    {
        _pUi->comboExpression->addItem(QStringLiteral("None selected"), QVariant(-1));

        for(qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
        {
            QString text = QString("%1 - %2").arg(_pGraphDataModel->registerAddress(idx))
                                             .arg(_pGraphDataModel->label(idx));

            _pUi->comboExpression->addItem(text, QVariant(idx));
        }
    }
    else
    {
        _pUi->comboExpression->addItem(QStringLiteral("No registers"), QVariant(-1));
    }

    _pUi->btnLoad->setEnabled(false);
    _pUi->btnSave->setEnabled(false);

    connect(_pUi->btnLoad, &QPushButton::clicked, this, &ExpressionsDialog::handleLoadExpression);
    connect(_pUi->btnSave, &QPushButton::clicked, this, &ExpressionsDialog::handleSaveExpression);
    connect(_pUi->btnClose, &QPushButton::clicked, this, &ExpressionsDialog::handleClose);

    connect(_pUi->comboExpression, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExpressionsDialog::handleRegisterSelected);

    _pUi->lineExpression->setText(_expressionParser.expression());
    connect(_pUi->lineExpression, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleExpressionChange);

    connect(_pUi->lineIn0, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn1, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);
    connect(_pUi->lineIn2, &QLineEdit::editingFinished, this, &ExpressionsDialog::handleInputChange);

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

void ExpressionsDialog::handleSaveExpression()
{
    bool result;
    int userDataIdx = _pUi->comboExpression->itemData(_pUi->comboExpression->currentIndex()).toInt(&result);

    if (result && userDataIdx >= 0)
    {
        _pGraphDataModel->setExpression(userDataIdx, _pUi->lineExpression->text());
    }
}

void ExpressionsDialog::handleLoadExpression()
{
    bool result;
    int userDataIdx = _pUi->comboExpression->itemData(_pUi->comboExpression->currentIndex()).toInt(&result);

    if (result && userDataIdx >= 0)
    {
        _pUi->lineExpression->setText(_pGraphDataModel->expression(userDataIdx));
        handleExpressionChange();
    }
}

void ExpressionsDialog::handleClose()
{
    done(QDialog::Accepted);
}

void ExpressionsDialog::handleRegisterSelected(int idx)
{
    bool result;
    int userDataIdx = _pUi->comboExpression->itemData(idx).toInt(&result);

    if (result)
    {
        if (userDataIdx >=0)
        {
            _pUi->btnLoad->setEnabled(true);
            _pUi->btnSave->setEnabled(true);
        }
        else
        {
            _pUi->btnLoad->setEnabled(false);
            _pUi->btnSave->setEnabled(false);
        }
    }
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
