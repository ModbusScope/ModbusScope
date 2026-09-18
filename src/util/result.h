#ifndef RESULT_H
#define RESULT_H

#include <QDebug>
#include <QFlags>
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

/*!
 * \brief The version of the generic adapter.* JSON-RPC contract this application speaks.
 *
 * Distinct from an adapter's own configVersion, which only versions that adapter's config
 * shape. adapter.initialize sends this in its params, adapter.describe must echo it in its
 * result; a mismatch means the adapter is incompatible and must not be used (see
 * AdapterClient). See docs/technical/adapter-protocol-spec.md for the full contract.
 */
constexpr int cProtocolVersion = 2;

/*!
 * \brief The generic data-quality vocabulary carried by every \c Result<T>.
 *
 * This is a hand-maintained copy of the contract defined by the adapter protocol (see
 * docs/technical/adapter-protocol-spec.md and, for the reference implementation, DummyAdapter's
 * shared/util/result.h, which ModbusAdapter and IEC104Adapter vendor directly). Keep the three
 * copies in sync when this vocabulary changes.
 *
 * \c State is the closed verdict every consumer must branch on; \c Flags is an open detail set
 * most consumers simply pass through. Keeping the two separate (rather than one packed value) is
 * what lets an adapter add a new flag id without a protocol version bump: an unrecognised flag id
 * is ignorable, but an unrecognised state is not (see stateFromId()).
 */
namespace DataQuality {

enum class State
{
    Good,     //!< A real measurement, no caveats.
    Degraded, //!< A real, usable measurement the source flagged (see Flags).
    Invalid,  //!< The source says do not use this value.
    NoValue   //!< No measurement has been received yet.
};

/*!
 * \brief Detail flags a source may attach to an otherwise-usable value.
 *
 * Flag ids are strings on the wire and an unknown id is always ignorable, so an adapter can add
 * members here without bumping the protocol version. The set an adapter may ever send is
 * declared in its adapter.describe response (capabilities.quality.flags).
 */
enum class Flag : quint32
{
    NoFlags = 0,
    Substituted = 1u << 0, //!< Value entered/forced at the source instead of measured.
    Blocked = 1u << 1,     //!< Value frozen/blocked for transmission at the source.
    Overflow = 1u << 2,    //!< Value exceeded the source's measuring range.
    OldData = 1u << 3,     //!< The source did not refresh the value within its own interval.
};
Q_DECLARE_FLAGS(Flags, Flag)

//! Wire id for \a state, e.g. State::Degraded -> "degraded".
QLatin1StringView stateId(State state);

//! Parses a wire state id; returns nullopt for anything unrecognised so a caller never falls
//! back to State::Good on unknown input.
std::optional<State> stateFromId(const QString& id);

//! Wire id for a single \a flag, e.g. Flag::OldData -> "oldData".
QLatin1StringView flagId(Flag flag);

//! Wire ids for every flag set in \a flags, in stable declaration order.
QStringList flagIds(Flags flags);

//! Parses wire flag ids into \a Flags. Any id not recognised is appended to \a unknownIds
//! (left untouched otherwise) and simply skipped, rather than rejecting the whole value.
Flags flagsFromIds(const QStringList& ids, QStringList& unknownIds);

/*!
 * \brief State and flags bundled together, for storage alongside a value (e.g. GraphSample)
 * once it has left its originating \c Result<T>.
 */
struct Quality
{
    State state = State::Good;
    Flags flags = Flag::NoFlags;

    friend bool operator==(const Quality& lhs, const Quality& rhs)
    {
        return lhs.state == rhs.state && lhs.flags == rhs.flags;
    }
};

} // namespace DataQuality
Q_DECLARE_OPERATORS_FOR_FLAGS(DataQuality::Flags)

/*!
 * \brief A value together with the data quality the source reported for it.
 *
 * \c state() is the closed verdict (Good/Degraded/Invalid/NoValue); \c flags() are the detail
 * bits a Good or Degraded value may carry (e.g. Substituted). isUsable() answers "may this
 * value be used" and is the general-purpose predicate call sites should branch on; hasValue()
 * answers the narrower "does this Result carry a real value at all", which is what wire
 * decoding needs to decide whether a value was sent. The two currently agree (both are true for
 * Good/Degraded only) but are named for their distinct call sites so they can diverge without a
 * rename if a future state ever carries a value without being usable.
 */
template <typename T>
class Result
{
public:
    Result();
    Result(T value, DataQuality::State state, DataQuality::Flags flags = DataQuality::Flag::NoFlags);
    Result(const Result<T>& copy);

    T value() const;
    void setValue(T value);

    void setError();

    bool isUsable() const;
    bool hasValue() const;

    DataQuality::State state() const;
    void setState(DataQuality::State state);

    DataQuality::Flags flags() const;

    //! Adds \a flags to the current set. Promotes State::Good to State::Degraded, but only when
    //! \a flags is non-empty, so a flag can never be added to a value silently still reported as
    //! unflagged Good — while a no-op call (e.g. always calling addFlags() with whatever detail
    //! flags a source reported, which may be none) never demotes an otherwise-clean Good result.
    //! Leaves Invalid/NoValue untouched since neither carries a usable value to degrade.
    void addFlags(DataQuality::Flags flags);

    //! Returns state() and flags() bundled together, for storage outside this Result.
    DataQuality::Quality quality() const;

    Result<T>& operator=(Result<T> const& result);

    friend bool operator==(const Result<T>& res1, const Result<T>& res2)
    {
        if ((res1._value == res2._value) && (res1._state == res2._state) && (res1._flags == res2._flags))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    T _value;
    DataQuality::State _state;
    DataQuality::Flags _flags;
};

/* Implementations need to be in header */

template <class T>
Result<T>::Result() : Result(0, DataQuality::State::NoValue)
{
}

template <class T>
Result<T>::Result(T value, DataQuality::State state, DataQuality::Flags flags)
    : _value(value), _state(state), _flags(flags)
{
}

template <class T>
Result<T>::Result(const Result<T>& copy) : _value(copy._value), _state(copy._state), _flags(copy._flags)
{
}

template <class T>
T Result<T>::value() const
{
    return _value;
}

template <class T>
void Result<T>::setValue(T value)
{
    _value = value;
    _state = DataQuality::State::Good;
}

template <class T>
void Result<T>::setError()
{
    _value = 0;
    _state = DataQuality::State::Invalid;
    _flags = DataQuality::Flag::NoFlags;
}

template <class T>
bool Result<T>::isUsable() const
{
    return _state == DataQuality::State::Good || _state == DataQuality::State::Degraded;
}

template <class T>
bool Result<T>::hasValue() const
{
    return _state == DataQuality::State::Good || _state == DataQuality::State::Degraded;
}

template <class T>
DataQuality::State Result<T>::state() const
{
    return _state;
}

template <class T>
void Result<T>::setState(DataQuality::State state)
{
    _state = state;
}

template <class T>
DataQuality::Flags Result<T>::flags() const
{
    return _flags;
}

template <class T>
void Result<T>::addFlags(DataQuality::Flags flags)
{
    _flags |= flags;
    if (flags && _state == DataQuality::State::Good)
    {
        _state = DataQuality::State::Degraded;
    }
}

template <class T>
DataQuality::Quality Result<T>::quality() const
{
    return DataQuality::Quality{ _state, _flags };
}

template <class T>
Result<T>& Result<T>::operator=(Result<T> const& result)
{
    // self-assignment guard
    if (this == &result)
    {
        return *this;
    }

    _value = result._value;
    _state = result._state;
    _flags = result._flags;

    // return the existing object so we can chain this operator
    return *this;
}

template <class T>
QDebug operator<<(QDebug debug, const Result<T>& result)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << '(' << DataQuality::stateId(result.state()).toString() << ", " << result.value()
                              << ')';

    return debug;
}

using ResultDouble = Result<double>;
using ResultDoubleList = QList<Result<double>>;

#endif // RESULT_H
