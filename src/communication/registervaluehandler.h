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

    void setRegisters(QList<ModbusRegister> &registerList);

    void startRead();
    void processPartialResult(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId);
    void finishRead();

    void registerAddresList(QList<quint16>& registerList, quint8 connectionId);

signals:
    void registerDataReady(QList<ModbusResult> registers);

private:

    uint32_t convertEndianness(bool bLittleEndian, uint16_t value, uint16_t nextValue);

    SettingsModel* _pSettingsModel;

    QList<ModbusRegister> _registerList;
    QList<ModbusResult> _resultList;
};

#endif // REGISTERVALUEHANDLER_H
