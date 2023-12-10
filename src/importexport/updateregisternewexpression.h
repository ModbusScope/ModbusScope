#ifndef UPDATE_REGISTER_NEW_EXPRESSION_H__
#define UPDATE_REGISTER_NEW_EXPRESSION_H__

#include <QString>

#include "modbusdatatype.h"

namespace UpdateRegisterNewExpression
{
    QString typeSuffix(ModbusDataType::Type type);
    QString constructRegisterString(quint32 registerAddress, ModbusDataType::Type type, quint8 connectionId);
}

#endif // UPDATE_REGISTER_NEW_EXPRESSION_H__

