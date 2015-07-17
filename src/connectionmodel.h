#ifndef CONNECTIONMODEL_H
#define CONNECTIONMODEL_H

#include <QObject>
#include <QString>
#include <QList>

class ConnectionModel : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionModel(QObject *parent = 0);
    ~ConnectionModel();

    void clearData();

    void setIpAddress(QString ip);
    void setPort(quint16 port);
    void setSlaveId(quint8 id);
    void setTimeout(quint32 timeout);

    QString ipAddress();
    quint16 port();
    quint8 slaveId();
    quint32 timeout();

signals:
    void ipChanged();
    void portChanged();
    void slaveIdChanged();
    void timeoutChanged();

private:
    QString _ipAddress;
    quint16 _port;
    quint8 _slaveId;
    quint32 _timeout;

};

#endif // CONNECTIONMODEL_H
