#ifndef COMMUNICATIONSTATSMODEL_H
#define COMMUNICATIONSTATSMODEL_H

#include <QObject>

class CommunicationStatsModel : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationStatsModel(QObject* parent = nullptr);

    qint64 startTime() const;
    qint64 endTime() const;
    quint32 successCount() const;
    quint32 errorCount() const;
    quint32 medianPollTime() const;
    qint64 runTime() const;

    void setStartTime(qint64 startTime);
    void setEndTime(qint64 endTime);
    void setStats(quint32 successCount, quint32 errorCount);
    void setMedianPollTime(quint32 pollTime);

signals:
    void statsChanged();
    void timeStatsChanged();

private:
    qint64 _startTime;
    qint64 _endTime;
    quint32 _successCount;
    quint32 _errorCount;
    quint32 _medianPollTime;
};

#endif // COMMUNICATIONSTATSMODEL_H
