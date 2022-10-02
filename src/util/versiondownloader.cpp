#include "versiondownloader.h"

#include <QUrl>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

VersionDownloader::VersionDownloader(QObject *parent) :
    QObject(parent)
{
    connect(&_webCtrl, &QNetworkAccessManager::finished, this, &VersionDownloader::updateManifestDownloaded);
}

VersionDownloader::~VersionDownloader()
{
}

void VersionDownloader::performCheck()
{
    QUrl versionManifest = QUrl("https://api.github.com/repos/jgeudens/ModbusScope/releases/latest");

    // Start request
    _webCtrl.get(QNetworkRequest(versionManifest));
}

void VersionDownloader::updateManifestDownloaded(QNetworkReply* pReply)
{
    if (pReply->error() == QNetworkReply::NoError)
    {
        QJsonDocument document = QJsonDocument::fromJson(pReply->readAll());
        QJsonObject rootObj = document.object();

        _publishDate = rootObj["published_at"].toString();
        _version = rootObj["tag_name"].toString();
        _url = rootObj["html_url"].toString();

        emit versionDownloaded();
    }

    // mark reply for deletion
    pReply->deleteLater();
}

QString VersionDownloader::version()
{
    return _version;
}

QString VersionDownloader::url()
{
    return _url;
}

QString VersionDownloader::publishDate()
{
    return _publishDate;
}
