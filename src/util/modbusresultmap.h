#ifndef MODBUSRESULTMAP_H
#define MODBUSRESULTMAP_H

#include "util/modbusdataunit.h"
#include "util/result.h"
#include <QMap>

using ModbusResultMap = QMap<ModbusDataUnit, Result<quint16>>;

#endif // MODBUSRESULTMAP_H
