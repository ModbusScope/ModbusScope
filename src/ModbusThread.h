#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

#include <QObject>
#include <QThread>
#include <ModbusMaster.h>

class ModbusThread : public QObject {
    Q_OBJECT

public:
    ModbusThread();
    ~ModbusThread();

    void start();
    void wait();

public slots:
    void stopThread();
    void startCommunication(/*QList<unsigned short> registers*/);
    void stopCommunication();

signals:
    void done();
    void error(QString err);

    void modbusResults(unsigned short result1, unsigned short result2/*QList<unsigned short> values*/);

private slots:
    void threadStopped();
    void threadStarted();

private:

    volatile bool bStop;
    volatile bool bActive;

    QThread *thread;

    ModbusMaster modbusMaster;
    QList<unsigned short> registerList;

    void process();

};

#endif // MODBUSTHREAD_H
