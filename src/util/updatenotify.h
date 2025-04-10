#ifndef UPDATENOTIFY_H
#define UPDATENOTIFY_H

#include "qdatetime.h"
#include <QObject>
#include <QUrl>

/* Forward declaration */
class VersionDownloader;

class UpdateNotify : public QObject
{
    Q_OBJECT
public:

    typedef enum
    {
        VERSION_LATEST = 0,             /* Latest version */
        VERSION_UPDATE_AVAILABLE,       /* Update available */
    } UpdateState;

    explicit UpdateNotify(VersionDownloader* pVersionDownloader, QString currentVersion, QObject *parent = nullptr);

    void checkForUpdate();

    QString version() const;
    QUrl link() const;
    bool bValidData() const;

    UpdateNotify::UpdateState versionState() const;

signals:
    void updateCheckResult(UpdateNotify::UpdateState result);

private slots:
    void handleVersionData();

private:

    UpdateState checkVersions(QString current, QString latest);
    UpdateState checkDate(QDate publishDate);

    VersionDownloader* _pVersionDownloader;

    QString _currentVersion;

    QString _version;
    QUrl _link;
    QDate _publishDate;
    bool _bValidData;

    UpdateNotify::UpdateState _versionState;

    static const int32_t _cdaysDelay = 14;

};

#endif // UPDATENOTIFY_H
