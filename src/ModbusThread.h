#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

#include <QObject>
#include <QThread>
#include <ModbusMaster.h>
#include <modbussettings.h>


class ModbusThread : public QObject {
    Q_OBJECT

public:
    ModbusThread();
    ~ModbusThread();

    void SetSettingsPointer(ModbusSettings * settings);

    void start();
    void wait();

public slots:
    void stopThread();
    void startCommunication();
    void stopCommunication();

signals:
    void done();
    void error(QString err);

    void modbusResults(QList<u_int16_t> values);

private slots:
    void threadStopped();
    void threadStarted();

private:

    volatile bool bStop;
    volatile bool bActive;

    QThread *thread;
    ModbusSettings *settings;

    ModbusMaster modbusMaster;
    QList<unsigned short> registerList;

    void process();

};

#endif // MODBUSTHREAD_H
