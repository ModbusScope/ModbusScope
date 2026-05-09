
#include "communicationstatsmodel.h"

#include <QDateTime>

/*!
 * \brief Constructs the model with all stats initialized to zero.
 */
CommunicationStatsModel::CommunicationStatsModel(QObject* parent)
    : QObject(parent), _startTime(0), _endTime(0), _successCount(0), _errorCount(0), _medianPollTime(0)
{
}

qint64 CommunicationStatsModel::startTime() const
{
    return _startTime;
}

qint64 CommunicationStatsModel::endTime() const
{
    return _endTime;
}

quint32 CommunicationStatsModel::successCount() const
{
    return _successCount;
}

quint32 CommunicationStatsModel::errorCount() const
{
    return _errorCount;
}

quint32 CommunicationStatsModel::medianPollTime() const
{
    return _medianPollTime;
}

/*!
 * \brief Returns the elapsed time in milliseconds since the communication start time.
 */
qint64 CommunicationStatsModel::runTime() const
{
    return QDateTime::currentMSecsSinceEpoch() - _startTime;
}

void CommunicationStatsModel::setStartTime(qint64 startTime)
{
    if (_startTime != startTime)
    {
        _startTime = startTime;
    }
}

void CommunicationStatsModel::setEndTime(qint64 endTime)
{
    if (_endTime != endTime)
    {
        _endTime = endTime;
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

void CommunicationStatsModel::setMedianPollTime(quint32 pollTime)
{
    _medianPollTime = pollTime;
}
