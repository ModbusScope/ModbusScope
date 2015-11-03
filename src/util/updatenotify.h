#ifndef UPDATENOTIFY_H
#define UPDATENOTIFY_H

#include <QObject>
#include <QWidget>

#include "versiondownloader.h"
#include "updatenotify.h"

class UpdateNotify : public QObject
{
    Q_OBJECT
public:
    explicit UpdateNotify(QObject *parent = 0);

    void checkForUpdate();

    QString version() const;
    QString link() const;
    quint32 dataLevel() const;
    bool bValidData() const;

    typedef enum
    {
        LATEST = 0,             /* Latest version */
        REV_UPDATE,             /* Revision update */
        MINOR_MAJOR_UPDATE,     /* Minor/major update */
    } UpdateState;

signals:
    void updateCheckResult(UpdateNotify::UpdateState result, bool bDataLevelUpdate);

public slots:

private slots:
    void handleVersionData();

private:

    UpdateState checkVersions(QString current, QString latest);

    VersionDownloader _versionDownloader;

    QString _version;
    QString _link;
    quint32 _dataLevel;
    bool _bValidData;

};

#endif // UPDATENOTIFY_H
