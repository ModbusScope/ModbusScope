#ifndef UPDATE_REGISTER_NEW_EXPRESSION_H__
#define UPDATE_REGISTER_NEW_EXPRESSION_H__

#include <QString>

#include "projectfiledata.h"

namespace UpdateRegisterNewExpression
{
    QString typeSuffix(bool is32bit, bool bUnsigned);
    QString constructRegisterString(quint32 registerAddress, bool is32bit, bool bUnsigned, quint8 connectionId);
    void convert(ProjectFileData::RegisterSettings& regSettings, QString& resultExpr);
}

#endif // UPDATE_REGISTER_NEW_EXPRESSION_H__

