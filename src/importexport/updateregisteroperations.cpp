
#include "updateregisteroperations.h"

#include <QStringLiteral>
#include <stdlib.h>

namespace UpdateRegisterOperations
{
    void convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr)
    {
        /*
         * Bitmask
         * Shift
         * Multiply
         * Divide
         */
        bool defBitmask = false;
        bool defShift = false;
        bool defMultiply = false;
        bool defDivide = false;

        if (regSettings.bitmask != defaultBitmask)
        {
            defBitmask = true;
        }

        if (regSettings.shift != defaultShift)
        {
            defShift = true;
        }

        if (regSettings.multiplyFactor != defaultMultiplyFactor)
        {
            defMultiply = true;
        }

        if (regSettings.divideFactor != defaultDivideFactor)
        {
            defDivide = true;
        }



        QString operation = QStringLiteral("REG");

        if (defBitmask)
        {
            QString strBitmask = QString("0x%1").arg(QString::number(regSettings.bitmask, 16).toUpper());

            operation = QString("%1&%2").arg(operation).arg(strBitmask);

            /* Add parenthesis when other operations */
            if (defShift | defMultiply | defDivide)
            {
                operation = QString("(%1)").arg(operation);
            }
        }

        if (defShift)
        {
            if (regSettings.shift > 0)
            {
                operation = QString("%1<<%2").arg(operation).arg(regSettings.shift);
            }
            else
            {
                operation = QString("%1>>%2").arg(operation).arg(abs(regSettings.shift));
            }

            /* Add parenthesis when other operations */
            if (defMultiply | defDivide)
            {
                operation = QString("(%1)").arg(operation);
            }
        }

        if (defMultiply)
        {
            operation = QString("%1*%2").arg(operation).arg(regSettings.multiplyFactor);
        }

        if (defDivide)
        {
            operation = QString("%1/%2").arg(operation).arg(regSettings.divideFactor);
        }

        resultExpr = operation;
    }
}
