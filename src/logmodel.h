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

    quint32 writeDuringLog();

    void setWriteDuringLogPath(QString path);
    QString writeDuringLogPath();

public slots:
    void setWriteDuringLog(bool bState);

signals:
    void pollTimeChanged();
    void writeDuringLogChanged();
    void writeDuringLogPathChanged();

private:

    quint32 _pollTime;

    bool _bWriteDuringLog;
    QString _writeDuringLogPath;

};

#endif // LOGMODEL_H
