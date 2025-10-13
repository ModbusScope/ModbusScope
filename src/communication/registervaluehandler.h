#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "communication/modbusregister.h"
#include "models/connectiontypes.h"
#include "util/modbusresultmap.h"

class SettingsModel;

class RegisterValueHandler : public QObject
{
    Q_OBJECT
public:

    explicit RegisterValueHandler(SettingsModel *pSettingsModel);

    void setRegisters(QList<ModbusRegister> &registerList);

    void startRead();
    void processPartialResult(ModbusResultMap partialResultMap, ConnectionTypes::connectionId_t connectionId);
    void finishRead();

    void registerAddresListForConnection(QList<ModbusDataUnit>& registerList,
                                         ConnectionTypes::connectionId_t connectionId);

signals:
    void registerDataReady(ResultDoubleList registers);

private:
    SettingsModel* _pSettingsModel;

    QList<ModbusRegister> _registerList;
    ResultDoubleList _resultList;
};

#endif // REGISTERVALUEHANDLER_H
