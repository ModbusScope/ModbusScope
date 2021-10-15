#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "modbusresult.h"
#include "modbusregister.h"

class SettingsModel;

class RegisterValueHandler : public QObject
{
    Q_OBJECT
public:

    RegisterValueHandler(SettingsModel *pSettingsModel);

    void prepareForData(QList<ModbusRegister> &registerList);

    void startRead();
    void processPartialResult(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId);
    void finishRead();

    void registerAddresList(QList<quint16>& registerList, quint8 connectionId);

signals:
    void registerDataReady(QList<bool> successList, QList<double> values);

private:

    SettingsModel* _pSettingsModel;

    QList<double> _processedValues;
    QList<bool> _successList;

    QList<ModbusRegister> _registerList;
};

#endif // REGISTERVALUEHANDLER_H
