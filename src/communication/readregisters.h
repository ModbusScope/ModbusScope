#ifndef READREGISTERCOLLECTION_H
#define READREGISTERCOLLECTION_H

#include <QObject>

#include "communication/modbusresultmap.h"

class ModbusReadItem
{
public:
    ModbusReadItem(ModbusDataUnit const& address, quint8 count) : _address(address), _count(count)
    {
    }

    ModbusDataUnit address() const
    {
        return _address;
    }
    quint8 count() const
    {
        return _count;
    }

private:
    ModbusDataUnit _address;
    quint8 _count{};

};


class ReadRegisters
{
public:
    ReadRegisters();

    void resetRead(QList<ModbusDataUnit> registerList, quint16 consecutiveMax);

    bool hasNext();
    ModbusReadItem next();

    void addSuccess(ModbusDataUnit const& startRegister, QList<quint16> registerDataList);
    void addError();
    void addAllErrors();
    void splitNextToSingleReads();

    ModbusResultMap resultMap();

private:


    QList<ModbusReadItem> _readItemList;

    ModbusResultMap _resultMap;

};

#endif // READREGISTERCOLLECTION_H
