#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "result.h"
#include "modbusaddress.h"
#include <QMap>

using ModbusResultMap = QMap<ModbusAddress, Result<quint16>>;

#endif // MODBUSRESULTMAP_H
