#include "expressionreferences.h"

#include "util/expressionregex.h"

#include <QRegularExpression>

namespace ExpressionReferences {

/*!
 * \brief Returns every \c ${...} reference in \a expression, in order, duplicates included.
 *
 * Uses the same regular expressions as ExpressionParser, so both agree on what a reference is.
 * The device is taken from the \c @n part, Device::cFirstDeviceId when absent. A reference whose
 * content does not parse is still returned, with Device::cFirstDeviceId as device.
 * \param expression The expression text.
 * \return The references with their position in \a expression.
 */
QList<Reference> find(const QString& expression)
{
    static const QRegularExpression matchRegex(ExpressionRegex::cMatchRegister);
    static const QRegularExpression parseRegex(ExpressionRegex::cParseReg);

    QList<Reference> references;
    QRegularExpressionMatchIterator it = matchRegex.globalMatch(expression);
    while (it.hasNext())
    {
        const QRegularExpressionMatch match = it.next();
        const QString text = match.captured(0);

        deviceId_t deviceId = Device::cFirstDeviceId;
        const QRegularExpressionMatch parsed = parseRegex.match(text);
        if (parsed.hasMatch() && !parsed.captured(2).isEmpty())
        {
            bool ok = false;
            const uint parsedId = parsed.captured(2).toUInt(&ok);
            if (ok)
            {
                deviceId = static_cast<deviceId_t>(parsedId);
            }
        }

        references.append({ match.capturedStart(0), match.capturedLength(0), text, deviceId });
    }

    return references;
}

/*!
 * \brief Replace one reference in \a expression with \a newText.
 *
 * Returns \a expression unchanged when \a reference no longer matches the text at its position,
 * for example because the expression changed after find() was called.
 * \param expression The expression text.
 * \param reference  A reference returned by find() for \a expression.
 * \param newText    The text to put in its place.
 * \return The expression with the reference replaced.
 */
QString replace(const QString& expression, const Reference& reference, const QString& newText)
{
    if (reference.start < 0 || expression.mid(reference.start, reference.length) != reference.text)
    {
        return expression;
    }

    QString result = expression;
    result.replace(reference.start, reference.length, newText);
    return result;
}

} // namespace ExpressionReferences
