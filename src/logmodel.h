#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QObject>

class LogModel : public QObject
{
    Q_OBJECT
public:
    explicit LogModel(QObject *parent = 0);
    ~LogModel();

    void setPollTime(quint32 pollTime);
    quint32 pollTime();

signals:
    void pollTimeChanged();

private:

    quint32 _pollTime;


};

#endif // LOGMODEL_H
