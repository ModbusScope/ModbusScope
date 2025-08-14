#ifndef READREGISTERCOLLECTION_H
#define READREGISTERCOLLECTION_H

#include <QObject>

#include "util/modbusresultmap.h"

class ModbusReadItem
{
public:
    ModbusReadItem(ModbusAddress address, quint8 count) :
        _address(address), _count(count)
    {
    }

    ModbusAddress address() const
    {
        return _address;
    }
    quint8 count() const
    {
        return _count;
    }

private:
    ModbusAddress _address;
    quint8 _count{};

};


class ReadRegisters
{
public:
    ReadRegisters();

    void resetRead(QList<ModbusAddress> registerList, quint16 consecutiveMax);

    bool hasNext();
    ModbusReadItem next();

    void addSuccess(ModbusAddress startRegister, QList<quint16> registerDataList);
    void addError();
    void addAllErrors();
    void splitNextToSingleReads();

    ModbusResultMap resultMap();

private:


    QList<ModbusReadItem> _readItemList;

    ModbusResultMap _resultMap;

};

#endif // READREGISTERCOLLECTION_H
