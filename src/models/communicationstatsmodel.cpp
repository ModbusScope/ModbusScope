
#include "communicationstatsmodel.h"

#include <QDateTime>

/*!
 * \brief Constructs the model with all stats initialized to zero.
 */
CommunicationStatsModel::CommunicationStatsModel(QObject* parent)
    : QObject(parent), _startTime(0), _endTime(0), _successCount(0), _errorCount(0), _medianPollTime(0)
{
}

//! Returns the communication start time as milliseconds since the epoch.
qint64 CommunicationStatsModel::startTime() const
{
    return _startTime;
}

//! Returns the communication end time as milliseconds since the epoch.
qint64 CommunicationStatsModel::endTime() const
{
    return _endTime;
}

//! Returns the number of successful communication cycles.
quint32 CommunicationStatsModel::successCount() const
{
    return _successCount;
}

//! Returns the number of failed communication cycles.
quint32 CommunicationStatsModel::errorCount() const
{
    return _errorCount;
}

//! Returns the median poll interval in milliseconds.
quint32 CommunicationStatsModel::medianPollTime() const
{
    return _medianPollTime;
}

/*!
 * \brief Returns the elapsed time in milliseconds since the communication start time.
 *
 * Returns 0 if communication has not started yet (i.e. start time is unset).
 */
qint64 CommunicationStatsModel::runTime() const
{
    if (_startTime == 0)
    {
        return 0;
    }

    return QDateTime::currentMSecsSinceEpoch() - _startTime;
}

//! Sets the communication start time to \a startTime (milliseconds since epoch).
void CommunicationStatsModel::setStartTime(qint64 startTime)
{
    if (startTime != _startTime)
    {
        _startTime = startTime;
        emit timeStatsChanged();
    }
}

//! Sets the communication end time to \a endTime (milliseconds since epoch).
void CommunicationStatsModel::setEndTime(qint64 endTime)
{
    if (endTime != _endTime)
    {
        _endTime = endTime;
        emit timeStatsChanged();
    }
}

/*!
 * \brief Updates success and error counts, emitting statsChanged() if either value differs.
 */
void CommunicationStatsModel::setStats(quint32 successCount, quint32 errorCount)
{
    if ((_successCount != successCount) || (_errorCount != errorCount))
    {
        _successCount = successCount;
        _errorCount = errorCount;
        emit statsChanged();
    }
}

//! Sets the median poll interval to \a pollTime milliseconds.
void CommunicationStatsModel::setMedianPollTime(quint32 pollTime)
{
    if (pollTime != _medianPollTime)
    {
        _medianPollTime = pollTime;
        emit timeStatsChanged();
    }
}
