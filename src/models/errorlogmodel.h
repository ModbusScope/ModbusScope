#ifndef ERRORLOGMODEL_H
#define ERRORLOGMODEL_H

#include <QObject>
#include "errorlog.h"

class ErrorLogModel : public QObject
{
    Q_OBJECT
public:
    explicit ErrorLogModel(QObject *parent = nullptr);

    void addError(QString message);
    void addInfo(QString message);

signals:

public slots:

private:

    QList<ErrorLog> _errorLogList;
};

#endif // ERRORLOGMODEL_H
