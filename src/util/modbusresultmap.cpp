
#include "modbusresultmap.h"
#include "modbusaddress.h"

void ModbusResultMap::insert(ModbusAddress key, Result<quint16> result)
{
    _resultMap.insert(key, result);
}

bool ModbusResultMap::contains(const ModbusAddress &key) const
{
    return _resultMap.contains(key);
}

qsizetype ModbusResultMap::size() const
{
    return _resultMap.size();
}

Result<quint16> ModbusResultMap::value(const ModbusAddress &key) const
{
    return _resultMap.value(key);
}

void ModbusResultMap::clear()
{
    _resultMap.clear();
}

// Implementation of [] operator.  This function must return a
// reference as array element can be put on left side
Result<quint16>& ModbusResultMap::operator[](quint32 index)
{
    return _resultMap[index];
}

QDebug operator<<(QDebug debug, const ModbusResultMap &map)
{
    Q_UNUSED(debug);
    Q_UNUSED(map);

    return debug;
    /*
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << ;

    return debug;
*/
}
