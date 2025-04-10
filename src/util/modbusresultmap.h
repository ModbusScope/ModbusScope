#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "util/modbusaddress.h"
#include "util/result.h"
#include <QMap>

using ModbusResultMap = QMap<ModbusAddress, Result<quint16>>;

#endif // MODBUSRESULTMAP_H
