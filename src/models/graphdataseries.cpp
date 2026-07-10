#include "graphdataseries.h"

#include <algorithm>

static bool lessThanTimestamp(const GraphSample& sample, double timestamp)
{
    return sample.timestamp < timestamp;
}

static bool timestampLessThan(double timestamp, const GraphSample& sample)
{
    return timestamp < sample.timestamp;
}

GraphDataSeries::GraphDataSeries() = default;

/*!
 * \brief Returns the number of samples in the series.
 */
qsizetype GraphDataSeries::size() const
{
    return _samples.size();
}

/*!
 * \brief Returns whether the series contains no samples.
 */
bool GraphDataSeries::isEmpty() const
{
    return _samples.isEmpty();
}

/*!
 * \brief Removes all samples from the series.
 */
void GraphDataSeries::clear()
{
    _samples.clear();
}

/*!
 * \brief Adds a sample to the series, keeping the samples sorted by timestamp.
 *
 * Appending a sample with a timestamp at or beyond the current last sample (the live polling case) is fast;
 * out-of-order samples are inserted at the correct position. Duplicate timestamps are allowed.
 *
 * \param sample Sample to add.
 */
void GraphDataSeries::add(const GraphSample& sample)
{
    if (_samples.isEmpty() || sample.timestamp >= _samples.last().timestamp)
    {
        _samples.append(sample);
    }
    else
    {
        auto it = std::lower_bound(_samples.begin(), _samples.end(), sample.timestamp, lessThanTimestamp);
        _samples.insert(it, sample);
    }
}

/*!
 * \brief Adds a sample to the series, keeping the samples sorted by timestamp.
 * \param timestamp Timestamp of the sample.
 * \param value Value of the sample.
 */
void GraphDataSeries::add(double timestamp, double value)
{
    add(GraphSample{timestamp, value});
}

/*!
 * \brief Replaces all samples with the supplied data, which is assumed to be sorted by timestamp.
 * \param timestamps Sample timestamps in ascending order.
 * \param values Sample values; must have the same size as \p timestamps.
 */
void GraphDataSeries::setSamples(const QList<double>& timestamps, const QList<double>& values)
{
    Q_ASSERT(timestamps.size() == values.size());

    const qsizetype count = qMin(timestamps.size(), values.size());

    _samples.clear();
    _samples.reserve(count);
    for (qsizetype idx = 0; idx < count; idx++)
    {
        _samples.append(GraphSample{timestamps[idx], values[idx]});
    }
}

/*!
 * \brief Returns a mutable iterator to the first sample.
 */
GraphDataSeries::iterator GraphDataSeries::begin()
{
    return _samples.begin();
}

/*!
 * \brief Returns a mutable iterator to one past the last sample.
 */
GraphDataSeries::iterator GraphDataSeries::end()
{
    return _samples.end();
}

/*!
 * \brief Returns a const iterator to the first sample.
 */
GraphDataSeries::const_iterator GraphDataSeries::constBegin() const
{
    return _samples.constBegin();
}

/*!
 * \brief Returns a const iterator to one past the last sample.
 */
GraphDataSeries::const_iterator GraphDataSeries::constEnd() const
{
    return _samples.constEnd();
}

/*!
 * \brief Returns an iterator to the first sample with a timestamp at or above \p timestamp.
 *
 * When \p expandedRange is true, the sample just below \p timestamp is included as well (clamped at the
 * first sample). Returns constEnd() when the series is empty.
 *
 * \param timestamp Timestamp to search for.
 * \param expandedRange Include the sample just below \p timestamp.
 * \return Iterator to the first sample of the range.
 */
GraphDataSeries::const_iterator GraphDataSeries::findBegin(double timestamp, bool expandedRange) const
{
    if (isEmpty())
    {
        return constEnd();
    }

    const_iterator it = std::lower_bound(constBegin(), constEnd(), timestamp, lessThanTimestamp);
    if (expandedRange && it != constBegin())
    {
        --it;
    }
    return it;
}

/*!
 * \brief Returns an iterator to one past the last sample with a timestamp at or below \p timestamp.
 *
 * When \p expandedRange is true, the sample just above \p timestamp is included as well (clamped at
 * constEnd()). Returns constEnd() when the series is empty.
 *
 * \param timestamp Timestamp to search for.
 * \param expandedRange Include the sample just above \p timestamp.
 * \return Iterator to one past the last sample of the range.
 */
GraphDataSeries::const_iterator GraphDataSeries::findEnd(double timestamp, bool expandedRange) const
{
    if (isEmpty())
    {
        return constEnd();
    }

    const_iterator it = std::upper_bound(constBegin(), constEnd(), timestamp, timestampLessThan);
    if (expandedRange && it != constEnd())
    {
        ++it;
    }
    return it;
}

/*!
 * \brief Returns the timestamp range spanned by the samples.
 *
 * Since the samples are sorted, the range is simply the first and last timestamp.
 *
 * \param bValid Set to false when the series is empty (the returned range is meaningless).
 * \return Minimum and maximum timestamp of the series.
 */
GraphDataSeries::TimestampRange GraphDataSeries::timestampRange(bool& bValid) const
{
    bValid = !isEmpty();

    if (!bValid)
    {
        return TimestampRange{0, 0};
    }

    return TimestampRange{_samples.first().timestamp, _samples.last().timestamp};
}
