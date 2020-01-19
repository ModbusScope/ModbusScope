#ifndef UPDATENOTIFY_H
#define UPDATENOTIFY_H

#include <QObject>
#include <QWidget>

#include "versiondownloader.h"

class UpdateNotify : public QObject
{
    Q_OBJECT
public:
    explicit UpdateNotify(QObject *parent = 0);

    void checkForUpdate();

    QString version() const;
    QString link() const;
    bool bValidData() const;

    typedef enum
    {
        VERSION_LATEST = 0,             /* Latest version */
        VERSION_UPDATE_AVAILABLE,       /* Update available */
    } UpdateState;

signals:
    void updateCheckResult(UpdateNotify::UpdateState result);

public slots:

private slots:
    void handleVersionData();

private:

    UpdateState checkVersions(QString current, QString latest);

    VersionDownloader _versionDownloader;

    QString _version;
    QString _link;
    bool _bValidData;

};

#endif // UPDATENOTIFY_H
