#ifndef EXPRESSIONREFERENCES_H
#define EXPRESSIONREFERENCES_H

#include "models/device.h"

#include <QList>
#include <QString>

namespace ExpressionReferences {

struct Reference
{
    qsizetype start;
    qsizetype length;
    QString text;
    deviceId_t deviceId;
};

QList<Reference> find(const QString& expression);
QString replace(const QString& expression, const Reference& reference, const QString& newText);

} // namespace ExpressionReferences

#endif // EXPRESSIONREFERENCES_H
