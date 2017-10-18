#include "errorlogmodel.h"

ErrorLogModel::ErrorLogModel(QObject *parent) : QObject(parent)
{

}

void ErrorLogModel::addError(QString message)
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    _errorLogList.append(ErrorLog(ErrorLog::LOG_ERROR, timestamp, message));

#ifdef DEBUG
    qDebug() << _errorLogList.last();
#endif
}

void ErrorLogModel::addInfo(QString message)
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    _errorLogList.append(ErrorLog(ErrorLog::LOG_INFO, timestamp, message));

#ifdef DEBUG
    qDebug() << _errorLogList.last();
#endif
}
