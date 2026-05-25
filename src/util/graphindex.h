#ifndef GRAPHINDEX_H
#define GRAPHINDEX_H

#include <QtGlobal>
#include <QMetaType>

/*!
 * \brief Identifies a graph by its position in the full graph list (active and inactive).
 *
 * A default-constructed GraphIdx (or GraphIdx::NONE) means "no graph selected".
 * Never pass a GraphIdx where an ActiveIdx is expected or vice versa.
 */
struct GraphIdx
{
    static constexpr qint32 NONE = -1;
    qint32 v;

    constexpr GraphIdx() : v(NONE) {}
    constexpr explicit GraphIdx(qint32 value) : v(value) {}

    bool isValid() const { return v != NONE; }

    bool operator==(GraphIdx o) const { return v == o.v; }
    bool operator!=(GraphIdx o) const { return v != o.v; }
    bool operator<(GraphIdx o) const { return v < o.v; }
};

/*!
 * \brief Identifies a graph by its sequential slot in QCustomPlot (active graphs only, 0-based).
 *
 * A default-constructed ActiveIdx (or ActiveIdx::NONE) means "no active graph".
 * Never pass an ActiveIdx where a GraphIdx is expected or vice versa.
 */
struct ActiveIdx
{
    static constexpr qint32 NONE = -1;
    qint32 v;

    constexpr ActiveIdx() : v(NONE) {}
    constexpr explicit ActiveIdx(qint32 value) : v(value) {}

    bool isValid() const { return v != NONE; }

    bool operator==(ActiveIdx o) const { return v == o.v; }
    bool operator!=(ActiveIdx o) const { return v != o.v; }
    bool operator<(ActiveIdx o) const { return v < o.v; }
};

Q_DECLARE_METATYPE(GraphIdx)
Q_DECLARE_METATYPE(ActiveIdx)

#endif // GRAPHINDEX_H
