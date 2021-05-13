
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

        /*
         * Workaround because older versions always export bitmask even when set to default.
         * This wil cause issues with signed values. The workaround is to ignore the bitmask when
         * the default 16-bit bitmask is used in combination with a 16 bit register
        */
        if (
            (!regSettings.b32Bit) // 16 bit register
            && (regSettings.bitmask == default16bitBitmask)
        )
        {
            defBitmask = false;
        }
        else if (regSettings.bitmask != default32bitBitmask)
        {
            defBitmask = true;
        }
        else
        {
            defBitmask = false;
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



        QString operation = QStringLiteral("VAL");

        if (defBitmask)
        {
            QString strBitmask = QString("0x%1").arg(QString::number(regSettings.bitmask, 16).toUpper());

            operation = QString("%1&%2").arg(operation, strBitmask);

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
