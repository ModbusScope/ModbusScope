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
    volatile unsigned int Cmd;
    volatile bool bCommRunning;

    QThread *thread;

    ModbusMaster modbusMaster;
    QList<unsigned short> registerList;

    void process();
    unsigned int doModbusComm(void);

    static const unsigned int cCmdNone = 0;
    static const unsigned int cCmdStart = 1;
    static const unsigned int cCmdStop = 2;

};

#endif // MODBUSTHREAD_H
