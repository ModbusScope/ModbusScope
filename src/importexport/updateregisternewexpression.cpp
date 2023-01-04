
#include "updateregisternewexpression.h"

#include <QStringLiteral>

namespace UpdateRegisterNewExpression
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

    QString constructRegisterString(quint32 registerAddress, ModbusDataType::Type type, quint8 connectionId)
    {
        QString suffix = UpdateRegisterNewExpression::typeSuffix(type);
        QString connStr = connectionId != 0 ? QString("@%1").arg(connectionId + 1) : QString();

        return QString("${%1%2%3}").arg(registerAddress).arg(connStr, suffix);
    }

    void convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr)
    {
        bool bFloat = false;
        auto type = ModbusDataType::convertSettings(regSettings.b32Bit, regSettings.bUnsigned, bFloat);

        QString regStr = constructRegisterString(regSettings.address, type, regSettings.connectionId);
        QString valStr = QStringLiteral("VAL");

        if (regSettings.bExpression && regSettings.expression.contains(valStr))
        {
            resultExpr = regSettings.expression.replace(valStr, regStr);
        }
        else
        {
            resultExpr = regStr;
        }
    }
}
