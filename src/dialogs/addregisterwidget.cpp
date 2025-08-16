#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "models/settingsmodel.h"
#include "util/expressiongenerator.h"
#include "util/modbusaddress.h"
#include "util/modbusdatatype.h"

using Type = ModbusDataType::Type;
using ObjectType = ModbusAddress::ObjectType;

Q_DECLARE_METATYPE(ModbusDataType::Type)

AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel, QWidget *parent) :
    QWidget(parent),
    _pUi(new Ui::AddRegisterWidget),
    _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    _pUi->lineName->setFocus();
    _pUi->lineName->selectAll();

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (quint8 i = 0u; i < ConnectionId::ID_CNT; i++)
    {
        if (_pSettingsModel->connectionState(i))
        {
            _pUi->cmbConnection->addItem(QString(tr("Connection %1").arg(i + 1)), i);
        }
    }

    _pUi->cmbObjectType->addItem("Coil", QVariant::fromValue(ObjectType::COIL));
    _pUi->cmbObjectType->addItem("Discrete input", QVariant::fromValue(ObjectType::DISCRETE_INPUT));
    _pUi->cmbObjectType->addItem("Input register", QVariant::fromValue(ObjectType::INPUT_REGISTER));
    _pUi->cmbObjectType->addItem("Holding register", QVariant::fromValue(ObjectType::HOLDING_REGISTER));

    _pUi->cmbType->addItem(ModbusDataType::description(Type::UNSIGNED_16), QVariant::fromValue(Type::UNSIGNED_16));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::UNSIGNED_32), QVariant::fromValue(Type::UNSIGNED_32));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::SIGNED_16), QVariant::fromValue(Type::SIGNED_16));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::SIGNED_32), QVariant::fromValue(Type::SIGNED_32));
    _pUi->cmbType->addItem(ModbusDataType::description(Type::FLOAT_32), QVariant::fromValue(Type::FLOAT_32));

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleResultAccept);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    resetFields();
}

AddRegisterWidget::~AddRegisterWidget()
{
    delete _pUi;
}

void AddRegisterWidget::handleResultAccept()
{
    QString expression = generateExpression();
    GraphData graphData;

    graphData.setLabel(_pUi->lineName->text());

    if (_pUi->radioSecondary->isChecked())
    {
        graphData.setValueAxis(GraphData::VALUE_AXIS_SECONDARY);
    }
    else
    {
        graphData.setValueAxis(GraphData::VALUE_AXIS_PRIMARY);
    }

    graphData.setExpression(expression);

    emit graphDataConfigured(graphData);

    resetFields();
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->setText("Name of curve");
    _pUi->spinAddress->setValue(0);
    _pUi->cmbType->setCurrentIndex(0);
    _pUi->cmbObjectType->setCurrentIndex(3);
    _pUi->cmbConnection->setCurrentIndex(0);
    _pUi->radioPrimary->setChecked(true);
}

QString AddRegisterWidget::generateExpression()
{
    connectionId_t connectionId;
    Type type;
    ObjectType objectType;

    QVariant typeData = _pUi->cmbType->currentData();
    if (typeData.canConvert<Type>())
    {
        type = typeData.value<Type>();
    }
    else
    {
        type = Type::UNSIGNED_16;
    }

    QVariant objectTypeData = _pUi->cmbObjectType->currentData();
    if (objectTypeData.canConvert<ObjectType>())
    {
        objectType = objectTypeData.value<ObjectType>();
    }
    else
    {
        objectType = ObjectType::UNKNOWN;
    }

    auto registerAddr = ModbusAddress(static_cast<quint32>(_pUi->spinAddress->value()), objectType);

    // TODO: dev
    QVariant connData = _pUi->cmbConnection->currentData();
    if (connData.canConvert<quint8>())
    {
        connectionId = connData.value<quint8>();
    }
    else
    {
        connectionId = 0;
    }

    return ExpressionGenerator::constructRegisterString(registerAddr.fullAddress(), type, connectionId);
}
