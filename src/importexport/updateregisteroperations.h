#ifndef UPDATE_REGISTER_OPERATIONS
#define UPDATE_REGISTER_OPERATIONS

#include <QString>

#include "projectfiledata.h"

namespace UpdateRegisterOperations
{
    void convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr);

    const double defaultDivideFactor = 1;
    const double defaultMultiplyFactor = 1;
    const quint32 default32bitBitmask = 0xFFFFFFFF;
    const quint32 default16bitBitmask = 0xFFFF;
    const quint32 defaultShift = 0;

}

#endif // UPDATE_REGISTER_OPERATIONS

