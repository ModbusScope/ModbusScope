#ifndef TESTSLAVEMODBUS_H
#define TESTSLAVEMODBUS_H

#include <QModbusTcpServer>
#include <QUrl>

class TestSlaveModbus : public QModbusTcpServer
{
    Q_OBJECT
public:
    explicit TestSlaveModbus(QObject *parent = nullptr);
    ~TestSlaveModbus();

    bool connect(QUrl host, int slaveId);

signals:

public slots:

};

#endif // TESTSLAVEMODBUS_H
