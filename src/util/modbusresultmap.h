#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "result.h"
#include "modbusaddress.h"
#include <QObject>
#include <QMap>

class ModbusResultMap
{

public:

    ModbusResultMap() = default;

    void insert(ModbusAddress key, Result<quint16>);
    void clear();
    bool contains(const ModbusAddress &key) const;

    qsizetype size() const;

    Result<quint16> value(const ModbusAddress &key) const;

    Result<quint16>& operator[](quint32);

private:
    QMap<ModbusAddress, Result<quint16>> _resultMap;

};

QDebug operator<<(QDebug debug, const ModbusResultMap &reg);


#endif // MODBUSRESULTMAP_H
