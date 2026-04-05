#ifndef EXPRESSION_GENERATOR_H__
#define EXPRESSION_GENERATOR_H__

#include "models/device.h"

#include <QString>

namespace ExpressionGenerator {
QString typeSuffix(const QString& typeId);
QString objectTypeToAddressPrefix(const QString& objectType);
QString constructRegisterString(const QString& objectType, int address, const QString& typeId, deviceId_t devId);
QString constructRegisterString(const QString& rawAddress, const QString& typeId, deviceId_t devId);
} // namespace ExpressionGenerator

#endif // EXPRESSION_GENERATOR_H__
