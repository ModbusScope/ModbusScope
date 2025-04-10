#ifndef CONNECTIONFORM_H
#define CONNECTIONFORM_H

#include "models/settingsmodel.h"

#include <QWidget>

namespace Ui {
class ConnectionForm;
}

class ConnectionForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionForm(QWidget *parent = nullptr);
    ~ConnectionForm();

    void fillSettingsModel(SettingsModel* pSettingsModel, quint8 connectionId);

    void setConnectionType(Connection::type_t connectionType);

    void setPortName(QString portName);
    void setParity(QSerialPort::Parity parity);
    void setBaudrate(QSerialPort::BaudRate baudrate);
    void setDatabits(QSerialPort::DataBits databits);
    void setStopbits(QSerialPort::StopBits stopbits);

    void setIpAddress(QString ip);
    void setPort(quint16 port);

    void setSlaveId(quint8 id);
    void setTimeout(quint32 timeout);
    void setConsecutiveMax(quint8 max);
    void setInt32LittleEndian(bool int32LittleEndian);
    void setPersistentConnection(bool persistentConnection);

public slots:
    void setState(bool bEnabled);

private slots:
    void connTypeSelected();

private:

    void enableSpecificSettings();

    Ui::ConnectionForm* _pUi;
};

#endif // CONNECTIONFORM_H
