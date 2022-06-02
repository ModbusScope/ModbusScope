#include "addregisterdialog.h"
#include "settingsmodel.h"
#include "updateregisternewexpression.h"

#include "ui_addregisterdialog.h"

AddRegisterDialog::AddRegisterDialog(SettingsModel* pSettingsModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::AddRegisterDialog),
    _pSettingsModel(pSettingsModel),
    _graphData()
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();
    _pUi->lineName->selectAll();

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (quint8 i = 0u; i < Connection::ID_CNT; i++)
    {
        if (_pSettingsModel->connectionState(i))
        {
            _pUi->cmbConnection->addItem(QString(tr("Connection %1").arg(i + 1)), i);
        }
    }

    _bitGroup.setExclusive(true);
    _bitGroup.addButton(_pUi->radio16Bit);
    _bitGroup.addButton(_pUi->radio32Bit);

    _signedGroup.setExclusive(true);
    _signedGroup.addButton(_pUi->radioSigned);
    _signedGroup.addButton(_pUi->radioUnsigned);
}

AddRegisterDialog::~AddRegisterDialog()
{
    delete _pUi;
}

GraphData AddRegisterDialog::graphData()
{
    return _graphData;
}

void AddRegisterDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        QString expression = generateExpression();

        _graphData.setLabel(_pUi->lineName->text());
        _graphData.setExpression(expression);

        bValid = true;
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}

QString AddRegisterDialog::generateExpression()
{
    quint32 registerAddress;
    bool is32bit = false;
    bool bUnsigned = true;
    quint8 connectionId;

    registerAddress = static_cast<quint32>(_pUi->spinAddress->value());

    if (_pUi->radioSigned->isChecked())
    {
        bUnsigned = false;
    }

    if (_pUi->radio32Bit->isChecked())
    {
        is32bit = true;
    }

    QVariant data = _pUi->cmbConnection->currentData();
    if (data.canConvert<quint8>())
    {
        connectionId = data.value<quint8>();
    }
    else
    {
        connectionId = 0;
    }

    return UpdateRegisterNewExpression::constructRegisterString(registerAddress, is32bit, bUnsigned, connectionId);
}
