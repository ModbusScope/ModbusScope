
#include "updateregisteroperations.h"

#include <QStringLiteral>

namespace UpdateRegisterOperations
{
    bool convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr)
    {
        /*
         * Bitmask
         * Shift
         * Multiply
         * Divide
         */

        QString reg = QStringLiteral("REG");

#if 0
        double divideFactor;
        double multiplyFactor;
        quint32 bitmask;
        quint32 shift;

        divideFactor(1),
        multiplyFactor(1)
        bitmask(0xFFFFFFFF)
        shift(0)
#endif

        resultExpr = reg;

        return true;
    }
}
