#ifndef UPDATE_REGISTER_OPERATIONS
#define UPDATE_REGISTER_OPERATIONS

#include <QString>

#include "projectfiledata.h"

namespace UpdateRegisterOperations
{
    bool convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr);
}

#endif // UPDATE_REGISTER_OPERATIONS

