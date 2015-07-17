#include "logmodel.h"

LogModel::LogModel(QObject *parent) :
    QObject(parent)
{
    setPollTime(1000);
}

LogModel::~LogModel()
{

}

void LogModel::setPollTime(quint32 pollTime)
{
    if (_pollTime != pollTime)
    {
        _pollTime = pollTime;
        emit pollTimeChanged();
    }
}

quint32 LogModel::pollTime()
{
    return _pollTime;
}
