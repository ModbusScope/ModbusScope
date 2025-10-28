
#include "expressiongenerator.h"
#include "models/device.h"

#include <QStringLiteral>

namespace ExpressionGenerator
{
    QString typeSuffix(ModbusDataType::Type type)
    {
        QString suffix;
        if (type == ModbusDataType::Type::UNSIGNED_16)
        {
            suffix = QString();
        }
        else
        {
            suffix = QString(":%1").arg(ModbusDataType::typeString(type));
        }

        return suffix;
    }

    QString constructRegisterString(QString registerAddress, ModbusDataType::Type type, deviceId_t devId)
    {
        QString suffix = ExpressionGenerator::typeSuffix(type);
        QString connStr = devId != Device::cFirstDeviceId ? QString("@%1").arg(devId) : QString();

        return QString("${%1%2%3}").arg(registerAddress, connStr, suffix);
    }
}
