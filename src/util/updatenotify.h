#ifndef UPDATENOTIFY_H
#define UPDATENOTIFY_H

#include <QObject>
#include <QUrl>

/* Forward declaration */
class VersionDownloader;

class UpdateNotify : public QObject
{
    Q_OBJECT
public:
    explicit UpdateNotify(VersionDownloader* pVersionDownloader, QObject *parent = 0);

    void checkForUpdate();

    QString version() const;
    QUrl link() const;
    bool bValidData() const;

    typedef enum
    {
        VERSION_LATEST = 0,             /* Latest version */
        VERSION_UPDATE_AVAILABLE,       /* Update available */
    } UpdateState;

signals:
    void updateCheckResult(UpdateNotify::UpdateState result);

private slots:
    void handleVersionData();

private:

    UpdateState checkVersions(QString current, QString latest);

    VersionDownloader* _pVersionDownloader;

    QString _version;
    QUrl _link;
    bool _bValidData;

};

#endif // UPDATENOTIFY_H
