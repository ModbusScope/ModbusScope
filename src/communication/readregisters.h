#ifndef READREGISTERCOLLECTION_H
#define READREGISTERCOLLECTION_H

#include <QObject>

#include "result.h"

class ModbusReadItem
{
public:
    ModbusReadItem(quint32 address, quint8 count) :
        _address(address), _count(count)
    {
    }

    quint32 address(void) { return _address; }
    quint8 count(void) { return _count; }

private:
   quint32 _address{};
   quint8 _count{};

};


class ReadRegisters
{
public:
    ReadRegisters();

    void resetRead(QList<quint32> registerList, quint16 consecutiveMax);

    bool hasNext();
    ModbusReadItem next();

    void addSuccess(quint32 startRegister, QList<quint16> registerDataList);
    void addError();
    void addAllErrors();
    void splitNextToSingleReads();

    QMap<quint32, Result<quint16> > resultMap();

private:


    QList<ModbusReadItem> _readItemList;

    QMap<quint32, Result<quint16> > _resultMap;

};

#endif // READREGISTERCOLLECTION_H
