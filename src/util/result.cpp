#include "util/result.h"

namespace DataQuality {

/*!
 * \brief Returns the wire id for \a state, e.g. State::Degraded -> "degraded".
 */
QLatin1StringView stateId(State state)
{
    switch (state)
    {
    case State::Good:
        return QLatin1StringView("good");
    case State::Degraded:
        return QLatin1StringView("degraded");
    case State::Invalid:
        return QLatin1StringView("invalid");
    case State::NoValue:
        return QLatin1StringView("noValue");
    }

    Q_UNREACHABLE();
}

/*!
 * \brief Parses a wire state id.
 * \return The matching state, or nullopt if \a id is not recognised, so a caller never falls back to
 * State::Good on unknown input.
 */
std::optional<State> stateFromId(const QString& id)
{
    if (id == QLatin1StringView("good"))
    {
        return State::Good;
    }
    if (id == QLatin1StringView("degraded"))
    {
        return State::Degraded;
    }
    if (id == QLatin1StringView("invalid"))
    {
        return State::Invalid;
    }
    if (id == QLatin1StringView("noValue"))
    {
        return State::NoValue;
    }

    return std::nullopt;
}

/*!
 * \brief Returns the wire id for a single \a flag, e.g. Flag::OldData -> "oldData".
 */
QLatin1StringView flagId(Flag flag)
{
    switch (flag)
    {
    case Flag::NoFlags:
        return QLatin1StringView();
    case Flag::Substituted:
        return QLatin1StringView("substituted");
    case Flag::Blocked:
        return QLatin1StringView("blocked");
    case Flag::Overflow:
        return QLatin1StringView("overflow");
    case Flag::OldData:
        return QLatin1StringView("oldData");
    }

    Q_UNREACHABLE();
}

/*!
 * \brief Returns the wire ids of every flag set in \a flags, in stable declaration order.
 */
QStringList flagIds(Flags flags)
{
    static const QList<Flag> cOrderedFlags{ Flag::Substituted, Flag::Blocked, Flag::Overflow, Flag::OldData };

    QStringList ids;
    for (Flag flag : cOrderedFlags)
    {
        if (flags.testFlag(flag))
        {
            ids.append(flagId(flag));
        }
    }
    return ids;
}

/*!
 * \brief Parses wire flag ids into \a Flags.
 *
 * Any id not recognised is appended to \a unknownIds and otherwise skipped, so one unfamiliar
 * flag id never invalidates the rest of the set.
 *
 * \param ids The wire flag ids to parse.
 * \param unknownIds Appended with any id that did not match a known flag.
 * \return The recognised flags.
 */
Flags flagsFromIds(const QStringList& ids, QStringList& unknownIds)
{
    Flags flags = Flag::NoFlags;
    for (const QString& id : ids)
    {
        if (id == QLatin1StringView("substituted"))
        {
            flags |= Flag::Substituted;
        }
        else if (id == QLatin1StringView("blocked"))
        {
            flags |= Flag::Blocked;
        }
        else if (id == QLatin1StringView("overflow"))
        {
            flags |= Flag::Overflow;
        }
        else if (id == QLatin1StringView("oldData"))
        {
            flags |= Flag::OldData;
        }
        else
        {
            unknownIds.append(id);
        }
    }
    return flags;
}

//! Number of bits reserved for the state in a data file quality code; the flags sit above them.
static constexpr quint32 cExportStateBits = 4;
static constexpr quint32 cExportStateMask = (1u << cExportStateBits) - 1u;

/*!
 * \brief Encodes \a quality as the numeric quality code stored in a data file.
 *
 * The code is \c state \c | \c (flags \c << \c 4) with Good=0, Degraded=1, Invalid=2 and
 * NoValue=3. The mapping is explicit so the file format does not depend on the enum order.
 */
quint32 toExportCode(const Quality& quality)
{
    quint32 stateCode = 0;
    switch (quality.state)
    {
    case State::Good:
        stateCode = 0;
        break;
    case State::Degraded:
        stateCode = 1;
        break;
    case State::Invalid:
        stateCode = 2;
        break;
    case State::NoValue:
        stateCode = 3;
        break;
    }

    quint32 flagCode = 0;
    if (quality.flags.testFlag(Flag::Substituted))
    {
        flagCode |= 1u << 0;
    }
    if (quality.flags.testFlag(Flag::Blocked))
    {
        flagCode |= 1u << 1;
    }
    if (quality.flags.testFlag(Flag::Overflow))
    {
        flagCode |= 1u << 2;
    }
    if (quality.flags.testFlag(Flag::OldData))
    {
        flagCode |= 1u << 3;
    }

    return stateCode | (flagCode << cExportStateBits);
}

/*!
 * \brief Decodes a numeric quality code read from a data file.
 *
 * Unknown flag bits are ignored. An unknown state decodes as Invalid, so unrecognised input
 * never looks like a usable value. Good with flags is returned as Degraded.
 *
 * \param code The quality code as written by toExportCode().
 * \return The decoded quality.
 */
Quality fromExportCode(quint32 code)
{
    Quality quality;

    switch (code & cExportStateMask)
    {
    case 0:
        quality.state = State::Good;
        break;
    case 1:
        quality.state = State::Degraded;
        break;
    case 2:
        quality.state = State::Invalid;
        break;
    case 3:
        quality.state = State::NoValue;
        break;
    default:
        quality.state = State::Invalid;
        break;
    }

    const quint32 flagCode = code >> cExportStateBits;
    if (flagCode & (1u << 0))
    {
        quality.flags |= Flag::Substituted;
    }
    if (flagCode & (1u << 1))
    {
        quality.flags |= Flag::Blocked;
    }
    if (flagCode & (1u << 2))
    {
        quality.flags |= Flag::Overflow;
    }
    if (flagCode & (1u << 3))
    {
        quality.flags |= Flag::OldData;
    }

    if (quality.flags && quality.state == State::Good)
    {
        quality.state = State::Degraded;
    }

    return quality;
}

} // namespace DataQuality
