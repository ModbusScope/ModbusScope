#ifndef EXPRESSIONVALUE_H
#define EXPRESSIONVALUE_H

#include <QString>

namespace ExpressionValue {

double parse(const QString& text, bool* ok);

} // namespace ExpressionValue

#endif // EXPRESSIONVALUE_H
