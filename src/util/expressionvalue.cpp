#include "expressionvalue.h"

namespace ExpressionValue {

namespace {

/*!
 * \brief Parse the digits of an integer written with a \c 0x or \c 0b prefix.
 *
 * Only digits of \a base are accepted: no sign, whitespace or second prefix.
 * Values above 2^53 lose precision when converted to double.
 * \param digits The text after the prefix.
 * \param base   16 for \c 0x, 2 for \c 0b.
 * \param ok     Set to true when \a digits is a valid number in \a base.
 * \return The parsed value, 0 when invalid.
 */
double parsePrefixedDigits(const QString& digits, int base, bool* ok)
{
    *ok = false;
    if (digits.isEmpty())
    {
        return 0;
    }

    const QString validDigits = base == 16 ? QStringLiteral("0123456789abcdefABCDEF") : QStringLiteral("01");
    for (const QChar digit : digits)
    {
        if (!validDigits.contains(digit))
        {
            return 0;
        }
    }

    const qulonglong value = digits.toULongLong(ok, base);
    return *ok ? static_cast<double>(value) : 0;
}

/*!
 * \brief Parse a decimal number with a point or a comma as decimal separator.
 * \param text The trimmed text.
 * \param ok   Set to true when \a text is a valid number.
 * \return The parsed value, 0 when invalid.
 */
double parseDecimal(const QString& text, bool* ok)
{
    if (text.contains(QLatin1Char('.')) && text.contains(QLatin1Char(',')))
    {
        *ok = false;
        return 0;
    }

    QString pointText = text;
    pointText.replace(QLatin1Char(','), QLatin1Char('.'));
    return pointText.toDouble(ok);
}

} // namespace

/*!
 * \brief Parse an example value as typed by the user.
 *
 * Accepts decimal numbers and integers with a \c 0x (hexadecimal) or \c 0b (binary) prefix,
 * optionally preceded by a minus sign. A decimal number can use a point or a comma as decimal
 * separator, but not both, as in an expression.
 * \param text The text to parse.
 * \param ok   Set to true when \a text is a valid number, false otherwise; may be nullptr.
 * \return The parsed value, 0 when \a text is not a valid number.
 */
double parse(const QString& text, bool* ok)
{
    bool localOk = false;
    if (ok == nullptr)
    {
        ok = &localOk;
    }

    const QString trimmed = text.trimmed();
    const bool bNegative = trimmed.startsWith(QLatin1Char('-'));
    const QString unsignedText = bNegative ? trimmed.mid(1) : trimmed;

    const QString prefix = unsignedText.left(2).toLower();
    int base = 0;
    if (prefix == QStringLiteral("0x"))
    {
        base = 16;
    }
    else if (prefix == QStringLiteral("0b"))
    {
        base = 2;
    }

    if (base == 0)
    {
        return parseDecimal(trimmed, ok);
    }

    const double value = parsePrefixedDigits(unsignedText.mid(2), base, ok);
    return (bNegative && value != 0) ? -value : value;
}

} // namespace ExpressionValue
