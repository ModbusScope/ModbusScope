#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "result.h"
#include <QObject>
#include <QMap>

class ModbusResultMap
{

public:

    ModbusResultMap();

    void insert(quint32, Result<quint16>);
    void clear();
    bool contains(const quint32 &key) const;

    qsizetype size() const;

    Result<quint16> value(const quint32 &key) const;

    Result<quint16>& operator[](quint32);

private:
    QMap<quint32, Result<quint16>> _resultMap;

};

QDebug operator<<(QDebug debug, const ModbusResultMap &reg);


#endif // MODBUSRESULTMAP_H
