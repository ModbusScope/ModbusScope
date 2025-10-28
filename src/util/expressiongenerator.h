#ifndef EXPRESSION_GENERATOR_H__
#define EXPRESSION_GENERATOR_H__

#include <QString>

#include "modbusdatatype.h"
#include "models/device.h"

namespace ExpressionGenerator
{
    QString typeSuffix(ModbusDataType::Type type);
    QString constructRegisterString(QString registerAddress, ModbusDataType::Type type, deviceId_t devId);
}

#endif // EXPRESSION_GENERATOR_H__

