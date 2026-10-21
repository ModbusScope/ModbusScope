#include "expressionvalue.h"

namespace ExpressionValue {

/*!
 * \brief Parse an example value as typed by the user.
 * \param text The text to parse.
 * \param ok   Set to true when \a text is a valid number, false otherwise; may be nullptr.
 * \return The parsed value, 0 when \a text is not a valid number.
 */
double parse(const QString& text, bool* ok)
{
    return text.toDouble(ok);
}

} // namespace ExpressionValue
