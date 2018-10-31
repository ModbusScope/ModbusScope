#ifndef READREGISTERCOLLECTION_H
#define READREGISTERCOLLECTION_H

#include <QObject>

#include "modbusresult.h"

class ModbusReadItem
{
public:
    ModbusReadItem(quint16 address, quint8 count)
    {
        _address = address;
        _count = count;
    }

    quint16 address(void) { return _address; }
    quint16 count(void) { return _count; }

private:
   quint16 _address;
   quint8 _count;

};


class ReadRegisters
{
public:
    ReadRegisters();

    void resetRead(QList<quint16> registerList, quint16 consecutiveMax);

    bool hasNext();
    ModbusReadItem next();

    void addSuccess(quint16 startRegister, QList<quint16> registerDataList);
    void addError();
    void addAllErrors();
    void splitNextToSingleReads();

    QMap<quint16, ModbusResult> resultMap();

private:


    QList<ModbusReadItem> _readItemList;

    QMap<quint16, ModbusResult> _resultMap;

};

#endif // READREGISTERCOLLECTION_H
