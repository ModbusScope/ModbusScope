#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "result.h"
#include "modbusregister.h"

class SettingsModel;

class RegisterValueHandler : public QObject
{
    Q_OBJECT
public:

    RegisterValueHandler(SettingsModel *pSettingsModel);

    void setRegisters(QList<ModbusRegister> &registerList);

    void startRead();
    void processPartialResult(QMap<quint32, Result> partialResultMap, quint8 connectionId);
    void finishRead();

    void registerAddresList(QList<quint32>& registerList, quint8 connectionId);

signals:
    void registerDataReady(QList<Result> registers);

private:

    uint32_t convertEndianness(bool bLittleEndian, quint16 value, quint16 nextValue);

    SettingsModel* _pSettingsModel;

    QList<ModbusRegister> _registerList;
    QList<Result> _resultList;
};

#endif // REGISTERVALUEHANDLER_H
