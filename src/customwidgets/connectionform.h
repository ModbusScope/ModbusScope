#ifndef CONNECTIONFORM_H
#define CONNECTIONFORM_H

#include <QSerialPort>
#include <QWidget>

#include "models/connection.h"

namespace Ui {
class ConnectionForm;
}

class ConnectionForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionForm(QWidget* parent = nullptr);
    ~ConnectionForm();

    void fillSettingsModel(Connection* connData);

    void setConnectionType(ConnectionTypes::type_t connectionType);

    void setPortName(QString portName);
    void setParity(QSerialPort::Parity parity);
    void setBaudrate(QSerialPort::BaudRate baudrate);
    void setDatabits(QSerialPort::DataBits databits);
    void setStopbits(QSerialPort::StopBits stopbits);

    void setIpAddress(QString ip);
    void setPort(quint16 port);

    void setTimeout(quint32 timeout);
    void setPersistentConnection(bool persistentConnection);

    void setState(bool bEnabled);

private slots:
    void connTypeSelected();

private:
    void enableSpecificSettings();

    Ui::ConnectionForm* _pUi;
};

#endif // CONNECTIONFORM_H
