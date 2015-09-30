#include "versiondownloader.h"

#include <QUrl>
#include <QByteArray>

VersionDownloader::VersionDownloader(QObject *parent) :
    QObject(parent)
{
    connect(&_webCtrl, SIGNAL (finished(QNetworkReply*)), this, SLOT (updateManifestDownloaded(QNetworkReply*)));
}

VersionDownloader::~VersionDownloader()
{
}

void VersionDownloader::performCheck()
{
    QUrl versionManifest = QUrl("http://jgeudens.github.io/public/update/modbusscope-update-manifest");

    // Start request
    _webCtrl.get(QNetworkRequest(versionManifest));
}

void VersionDownloader::updateManifestDownloaded(QNetworkReply* pReply)
{
    if (pReply->error() == QNetworkReply::NoError)
    {
        _downloadedVersionData = pReply->readAll();

        emit versionDownloaded();
    }

    // mark reply for deletion
    pReply->deleteLater();
}

QByteArray VersionDownloader::downloadedVersionData()
{
    return _downloadedVersionData;
}
