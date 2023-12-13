#ifndef EXPRESSION_GENERATOR_H__
#define EXPRESSION_GENERATOR_H__

#include <QString>

#include "modbusdatatype.h"

namespace ExpressionGenerator
{
    QString typeSuffix(ModbusDataType::Type type);
    QString constructRegisterString(quint32 registerAddress, ModbusDataType::Type type, quint8 connectionId);
}

#endif // EXPRESSION_GENERATOR_H__

