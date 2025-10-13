#include "customwidgets/connectionform.h"
#include "ui_connectionform.h"

ConnectionForm::ConnectionForm(QWidget* parent) : QWidget(parent), _pUi(new Ui::ConnectionForm)
{
    _pUi->setupUi(this);

    _pUi->comboType->addItem("TCP", QVariant(ConnectionTypes::TYPE_TCP));
    _pUi->comboType->addItem("Serial", QVariant(ConnectionTypes::TYPE_SERIAL));
    _pUi->comboType->setCurrentIndex(0);
    connect(_pUi->comboType, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
            &ConnectionForm::connTypeSelected);

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
    _pUi->comboStopBits->addItem("One and half bits", QVariant(QSerialPort::OneAndHalfStop));
    _pUi->comboStopBits->addItem("Two bits", QVariant(QSerialPort::TwoStop));
    _pUi->comboStopBits->setCurrentIndex(0);

    connTypeSelected();
}

ConnectionForm::~ConnectionForm()
{
    delete _pUi;
}

void ConnectionForm::setState(bool bEnabled)
{
    _pUi->spinTimeout->setEnabled(bEnabled);
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
        _pUi->comboPortName->setEnabled(false);
        _pUi->comboBaud->setEnabled(false);
        _pUi->comboParity->setEnabled(false);
        _pUi->comboDataBits->setEnabled(false);
        _pUi->comboStopBits->setEnabled(false);
    }
}

void ConnectionForm::fillSettingsModel(Connection* connData)
{
    connData->setTimeout(_pUi->spinTimeout->value());
    connData->setPersistentConnection(_pUi->checkPersistentConn->checkState() == Qt::Checked);
    connData->setConnectionType(static_cast<ConnectionTypes::type_t>(_pUi->comboType->currentData().toUInt()));
    connData->setIpAddress(_pUi->lineIP->text());
    connData->setPort(_pUi->spinPort->value());
    connData->setPortName(_pUi->comboPortName->currentText());
    connData->setBaudrate(static_cast<QSerialPort::BaudRate>(_pUi->comboBaud->currentData().toUInt()));
    connData->setParity(static_cast<QSerialPort::Parity>(_pUi->comboParity->currentData().toUInt()));
    connData->setDatabits(static_cast<QSerialPort::DataBits>(_pUi->comboDataBits->currentData().toUInt()));
    connData->setStopbits(static_cast<QSerialPort::StopBits>(_pUi->comboStopBits->currentData().toUInt()));
}

void ConnectionForm::setConnectionType(ConnectionTypes::type_t connectionType)
{
    int index = _pUi->comboType->findData(QVariant(connectionType));
    if (index != -1)
    {
        _pUi->comboType->setCurrentIndex(index);
    }
}

void ConnectionForm::setPortName(QString portName)
{
    _pUi->comboPortName->setCurrentText(portName);
}

void ConnectionForm::setParity(QSerialPort::Parity parity)
{
    int index = _pUi->comboParity->findData(QVariant(parity));
    if (index != -1)
    {
        _pUi->comboParity->setCurrentIndex(index);
    }
}

void ConnectionForm::setBaudrate(QSerialPort::BaudRate baudrate)
{
    int index = _pUi->comboBaud->findData(QVariant(baudrate));
    if (index != -1)
    {
        _pUi->comboBaud->setCurrentIndex(index);
    }
}

void ConnectionForm::setDatabits(QSerialPort::DataBits databits)
{
    int index = _pUi->comboDataBits->findData(QVariant(databits));
    if (index != -1)
    {
        _pUi->comboDataBits->setCurrentIndex(index);
    }
}

void ConnectionForm::setStopbits(QSerialPort::StopBits stopbits)
{
    int index = _pUi->comboStopBits->findData(QVariant(stopbits));
    if (index != -1)
    {
        _pUi->comboStopBits->setCurrentIndex(index);
    }
}

void ConnectionForm::setIpAddress(QString ip)
{
    _pUi->lineIP->setText(ip);
}

void ConnectionForm::setPort(quint16 port)
{
    _pUi->spinPort->setValue(port);
}

void ConnectionForm::setTimeout(quint32 timeout)
{
    _pUi->spinTimeout->setValue(timeout);
}

void ConnectionForm::setPersistentConnection(bool persistentConnection)
{
    _pUi->checkPersistentConn->setChecked(persistentConnection);
}

void ConnectionForm::connTypeSelected()
{
    enableSpecificSettings();
}

void ConnectionForm::enableSpecificSettings()
{
    bool bTcp =
      static_cast<ConnectionTypes::type_t>(_pUi->comboType->currentData().toUInt()) == ConnectionTypes::TYPE_TCP;

    _pUi->lineIP->setEnabled(bTcp);
    _pUi->spinPort->setEnabled(bTcp);
    _pUi->comboPortName->setEnabled(!bTcp);
    _pUi->comboBaud->setEnabled(!bTcp);
    _pUi->comboParity->setEnabled(!bTcp);
    _pUi->comboDataBits->setEnabled(!bTcp);
    _pUi->comboStopBits->setEnabled(!bTcp);
}
