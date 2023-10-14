#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "result.h"
#include "modbusaddress.h"
#include <QMap>

typedef QMap<ModbusAddress, Result<quint16>> ModbusResultMap;

#endif // MODBUSRESULTMAP_H
