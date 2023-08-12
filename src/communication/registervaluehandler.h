#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "modbusresultmap.h"
#include "result.h"
#include "modbusregister.h"

class SettingsModel;

class RegisterValueHandler : public QObject
{
    Q_OBJECT
public:

    explicit RegisterValueHandler(SettingsModel *pSettingsModel);

    void setRegisters(QList<ModbusRegister> &registerList);

    void startRead();
    void processPartialResult(ModbusResultMap partialResultMap, quint8 connectionId);
    void finishRead();

    void registerAddresList(QList<ModbusAddress>& registerList, quint8 connectionId);

signals:
    void registerDataReady(ResultDoubleList registers);

private:
    SettingsModel* _pSettingsModel;

    QList<ModbusRegister> _registerList;
    ResultDoubleList _resultList;
};

#endif // REGISTERVALUEHANDLER_H
