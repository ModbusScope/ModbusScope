
#include "updateregisternewexpression.h"

#include <QStringLiteral>
#include <stdlib.h>

namespace UpdateRegisterNewExpression
{
    QString typeSuffix(bool is32bit, bool bUnsigned)
    {
        QString suffix;

        if (is32bit)
        {
            if (bUnsigned)
            {
                suffix = QStringLiteral(":32b");
            }
            else
            {
                suffix = QStringLiteral(":s32b");
            }
        }
        else if (!bUnsigned)
        {
            suffix = QStringLiteral(":s16b");
        }
        else
        {
            suffix = QString();
        }

        return suffix;
    }

    QString constructRegisterString(quint16 registerAddress, bool is32bit, bool bUnsigned, quint8 connectionId)
    {
        QString suffix = UpdateRegisterNewExpression::typeSuffix(is32bit, bUnsigned);
        QString connStr = connectionId != 0 ? QString("@%1").arg(connectionId + 1) : QString();

        return QString("${%1%2%3}").arg(registerAddress).arg(connStr, suffix);
    }

    void convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr)
    {
        QString regStr = constructRegisterString(regSettings.address, regSettings.b32Bit, regSettings.bUnsigned, regSettings.connectionId);
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
