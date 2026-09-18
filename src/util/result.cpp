#include "util/result.h"

namespace DataQuality {

/*!
 * \brief Returns the wire id for \a state.
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
 * \return The matching state, or nullopt if \a id is not recognised.
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
 * \brief Returns the wire id for a single \a flag.
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

} // namespace DataQuality
