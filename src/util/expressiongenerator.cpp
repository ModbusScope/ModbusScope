
#include "expressiongenerator.h"
#include "models/device.h"
#include "util/scopelogging.h"

#include <QStringLiteral>

namespace ExpressionGenerator {
/*!
 * \brief Returns the type suffix for an expression string.
 * \param typeId The data type identifier (e.g. "16b", "s16b", "f32b").
 * \return Empty string for the default type "16b", otherwise ":\a typeId".
 */
QString typeSuffix(const QString& typeId)
{
    if (typeId == QStringLiteral("16b") || typeId.isEmpty())
    {
        return QString();
    }
    return QString(":%1").arg(typeId);
}

/*!
 * \brief Returns the single-character address prefix for an object type string.
 * \param objectType One of "coil", "discrete-input", "input-register", "holding-register".
 * \return The corresponding prefix character ("c", "d", "i", or "h"),
 *         or an empty string if \a objectType is not a recognised value.
 */
QString objectTypeToAddressPrefix(const QString& objectType)
{
    if (objectType == QStringLiteral("coil"))
    {
        return QStringLiteral("c");
    }
    if (objectType == QStringLiteral("discrete-input"))
    {
        return QStringLiteral("d");
    }
    if (objectType == QStringLiteral("input-register"))
    {
        return QStringLiteral("i");
    }
    if (objectType == QStringLiteral("holding-register"))
    {
        return QStringLiteral("h");
    }
    qCWarning(scopeGeneralInfo) << "objectTypeToAddressPrefix: unknown objectType:" << objectType;
    return QString();
}

/*!
 * \brief Constructs a register expression string from its component parts.
 * \param objectType Object type string (e.g. "holding-register").
 * \param address    Zero-based register address within the object type space.
 * \param typeId     Data type identifier (e.g. "16b", "f32b").
 * \param devId      Device identifier; omitted from the string when it equals the first device ID.
 * \return A register expression string such as \c ${h0}, \c ${h5@2:f32b},
 *         or an empty string if \a objectType is not recognised.
 */
QString constructRegisterString(const QString& objectType, int address, const QString& typeId, deviceId_t devId)
{
    const QString prefix = objectTypeToAddressPrefix(objectType);
    if (prefix.isEmpty())
    {
        return QString();
    }
    const QString suffix = typeSuffix(typeId);
    const QString connStr = devId != Device::cFirstDeviceId ? QString("@%1").arg(devId) : QString();

    return QString("${%1%2%3%4}").arg(prefix).arg(address).arg(connStr, suffix);
}

/*!
 * \brief Constructs a register expression string from a pre-formatted address string.
 *
 * Use this overload when the address is already in the protocol format (e.g. "40001"
 * from an MBC file), rather than as a separate objectType and zero-based index.
 *
 * \param rawAddress Pre-formatted address string (e.g. "40001", "h0").
 * \param typeId     Data type identifier (e.g. "16b", "f32b").
 * \param devId      Device identifier; omitted from the string when it equals the first device ID.
 * \return A register expression string such as \c ${40001}, \c ${40001@2:f32b}.
 */
QString constructRegisterString(const QString& rawAddress, const QString& typeId, deviceId_t devId)
{
    const QString suffix = typeSuffix(typeId);
    const QString connStr = devId != Device::cFirstDeviceId ? QString("@%1").arg(devId) : QString();

    return QString("${%1%2%3}").arg(rawAddress, connStr, suffix);
}
} // namespace ExpressionGenerator
