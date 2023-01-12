#include "addregisterdialog.h"
#include "settingsmodel.h"
#include "updateregisternewexpression.h"
#include "modbusdatatype.h"

#include "ui_addregisterdialog.h"

using Type = ModbusDataType::Type;

Q_DECLARE_METATYPE(ModbusDataType::Type)

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

    _pUi->cmbType->addItem(ModbusDataType::description(Type::UNSIGNED_16), QVariant::fromValue(Type::UNSIGNED_16));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::UNSIGNED_32), QVariant::fromValue(Type::UNSIGNED_32));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::SIGNED_16), QVariant::fromValue(Type::SIGNED_16));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::SIGNED_32), QVariant::fromValue(Type::SIGNED_32));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::FLOAT_32), QVariant::fromValue(Type::FLOAT_32));

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);
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

        if (_pUi->radioSecondary->isChecked())
        {
            _graphData.setValueAxis(GraphData::VALUE_AXIS_SECONDARY);
        }
        else
        {
            _graphData.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
        }

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
    quint8 connectionId;
    Type type;

    registerAddress = static_cast<quint32>(_pUi->spinAddress->value());

    QVariant typeData = _pUi->cmbType->currentData();
    if (typeData.canConvert<Type>())
    {
        type = typeData.value<Type>();
    }
    else
    {
        type = Type::UNSIGNED_16;
    }

    QVariant connData = _pUi->cmbConnection->currentData();
    if (connData.canConvert<quint8>())
    {
        connectionId = connData.value<quint8>();
    }
    else
    {
        connectionId = 0;
    }

    return UpdateRegisterNewExpression::constructRegisterString(registerAddress, type, connectionId);
}
