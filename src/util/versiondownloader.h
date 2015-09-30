#ifndef VERSIONDOWNLOADER_H
#define VERSIONDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class VersionDownloader : public QObject
{
    Q_OBJECT
public:
    explicit VersionDownloader(QObject *parent = 0);
    virtual ~VersionDownloader();

    void performCheck();

    QByteArray downloadedVersionData();

signals:
    void versionDownloaded();

private slots:
    void updateManifestDownloaded(QNetworkReply* pReply);

private:

    QNetworkAccessManager _webCtrl;
    QByteArray _downloadedVersionData;

};

#endif // VERSIONDOWNLOADER_H
