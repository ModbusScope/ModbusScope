#include "connectionform.h"
#include "ui_connectionform.h"

#include "settingsmodel.h"

ConnectionForm::ConnectionForm(QWidget *parent) :
    QWidget(parent),
    _pUi(new Ui::ConnectionForm)
{
    _pUi->setupUi(this);

    _pUi->linePortName->setText(QStringLiteral("/dev/ttyUSB0"));

    _pUi->comboType->addItem("TCP", QVariant(SettingsModel::CONNECTION_TYPE_TCP));
    _pUi->comboType->addItem("Serial", QVariant(SettingsModel::CONNECTION_TYPE_SERIAL));
    _pUi->comboType->setCurrentIndex(0);
    connect(_pUi->comboType, QOverload<const int>::of(&QComboBox::currentIndexChanged), this, &ConnectionForm::connTypeSelected);

    _pUi->comboBaud->addItem("1200", QVariant(QSerialPort::Baud1200));
    _pUi->comboBaud->addItem("2400", QVariant(QSerialPort::Baud2400));
    _pUi->comboBaud->addItem("4800", QVariant(QSerialPort::Baud4800));
    _pUi->comboBaud->addItem("9600", QVariant(QSerialPort::Baud9600));
    _pUi->comboBaud->addItem("19200", QVariant(QSerialPort::Baud19200));
    _pUi->comboBaud->addItem("38400", QVariant(QSerialPort::Baud38400));
    _pUi->comboBaud->addItem("57600", QVariant(QSerialPort::Baud57600));
    _pUi->comboBaud->addItem("115200", QVariant(QSerialPort::Baud115200));
    _pUi->comboBaud->setCurrentIndex(7);

    _pUi->comboDataBits->addItem("5 bits", QVariant(QSerialPort::Data5));
    _pUi->comboDataBits->addItem("6 bits", QVariant(QSerialPort::Data6));
    _pUi->comboDataBits->addItem("7 bits", QVariant(QSerialPort::Data7));
    _pUi->comboDataBits->addItem("8 bits", QVariant(QSerialPort::Data8));
    _pUi->comboDataBits->setCurrentIndex(3);

    _pUi->comboParity->addItem("No parity", QVariant(QSerialPort::NoParity));
    _pUi->comboParity->addItem("Even parity", QVariant(QSerialPort::EvenParity));
    _pUi->comboParity->addItem("Odd parity", QVariant(QSerialPort::OddParity));
    _pUi->comboParity->setCurrentIndex(0);

    _pUi->comboStopBits->addItem("One bit", QVariant(QSerialPort::OneStop));
    _pUi->comboStopBits->addItem("Two bits", QVariant(QSerialPort::TwoStop));
    _pUi->comboStopBits->setCurrentIndex(0);
}

ConnectionForm::~ConnectionForm()
{
    delete _pUi;
}

void ConnectionForm::setState(bool bEnabled)
{
    _pUi->spinSlaveId->setEnabled(bEnabled);
    _pUi->spinTimeout->setEnabled(bEnabled);
    _pUi->spinConsecutiveMax->setEnabled(bEnabled);
    _pUi->checkInt32LittleEndian->setEnabled(bEnabled);
    _pUi->checkPersistentConn->setEnabled(bEnabled);

    _pUi->comboType->setEnabled(bEnabled);

    if (bEnabled)
    {
        /* Set interface settings based on connection */
        enableSpecificSettings();
    }
    else
    {
        _pUi->lineIP->setEnabled(false);
        _pUi->spinPort->setEnabled(false);
        _pUi->linePortName->setEnabled(false);
        _pUi->comboBaud->setEnabled(false);
        _pUi->comboParity->setEnabled(false);
        _pUi->comboDataBits->setEnabled(false);
        _pUi->comboStopBits->setEnabled(false);
    }
}

void ConnectionForm::fillSettingsModel(SettingsModel* pSettingsModel, quint8 connectionId)
{
    pSettingsModel->setSlaveId(connectionId, _pUi->spinSlaveId->value());
    pSettingsModel->setTimeout(connectionId, _pUi->spinTimeout->value());
    pSettingsModel->setConsecutiveMax(connectionId, _pUi->spinConsecutiveMax->value());
    pSettingsModel->setInt32LittleEndian(connectionId, _pUi->checkInt32LittleEndian->checkState() == Qt::Checked);
    pSettingsModel->setPersistentConnection(connectionId, _pUi->checkPersistentConn->checkState() == Qt::Checked);

    pSettingsModel->setConnectionType(connectionId, static_cast<SettingsModel::ConnectionType_t>(_pUi->comboType->currentData().toUInt()));

    pSettingsModel->setIpAddress(connectionId, _pUi->lineIP->text());
    pSettingsModel->setPort(connectionId, _pUi->spinPort->value());

    pSettingsModel->setPortName(connectionId, _pUi->linePortName->text());
    pSettingsModel->setBaudrate(connectionId, static_cast<QSerialPort::BaudRate>(_pUi->comboBaud->currentData().toUInt()));
    pSettingsModel->setParity(connectionId, static_cast<QSerialPort::Parity>(_pUi->comboParity->currentData().toUInt()));
    pSettingsModel->setDatabits(connectionId, static_cast<QSerialPort::DataBits>(_pUi->comboDataBits->currentData().toUInt()));
    pSettingsModel->setStopbits(connectionId, static_cast<QSerialPort::StopBits>(_pUi->comboStopBits->currentData().toUInt()));
}

void ConnectionForm::connTypeSelected(qint32 index)
{
    Q_UNUSED(index);

    enableSpecificSettings();
}

void ConnectionForm::enableSpecificSettings()
{
    bool bTcp = static_cast<SettingsModel::ConnectionType_t>(_pUi->comboType->currentData().toUInt()) == SettingsModel::CONNECTION_TYPE_TCP;

    _pUi->lineIP->setEnabled(bTcp);
    _pUi->spinPort->setEnabled(bTcp);
    _pUi->linePortName->setEnabled(!bTcp);
    _pUi->comboBaud->setEnabled(!bTcp);
    _pUi->comboParity->setEnabled(!bTcp);
    _pUi->comboDataBits->setEnabled(!bTcp);
    _pUi->comboStopBits->setEnabled(!bTcp);
}
