
#include "expressiongenerator.h"

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

    QString constructRegisterString(QString registerAddress, ModbusDataType::Type type, quint8 connectionId)
    {
        QString suffix = ExpressionGenerator::typeSuffix(type);
        QString connStr = connectionId != 0 ? QString("@%1").arg(connectionId + 1) : QString();

        return QString("${%1%2%3}").arg(registerAddress, connStr, suffix);
    }
}
