#include "logmodel.h"

LogModel::LogModel(QObject *parent) :
    QObject(parent)
{
    _pollTime = 1000;
    _bWriteDuringLog = false;
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

void LogModel::setWriteDuringLog(bool bState)
{
    if (_bWriteDuringLog != bState)
    {
        _bWriteDuringLog = bState;
        emit writeDuringLogChanged();
    }
}

quint32 LogModel::writeDuringLog()
{
    return _bWriteDuringLog;
}

void LogModel::setWriteDuringLogPath(QString path)
{
    if (_writeDuringLogPath != path)
    {
        _writeDuringLogPath = path;
        emit writeDuringLogPathChanged();
    }
}

QString LogModel::writeDuringLogPath()
{
    return _writeDuringLogPath;
}



