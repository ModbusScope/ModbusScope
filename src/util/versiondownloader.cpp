#include "versiondownloader.h"

#include "util/util.h"
#include "util/version.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QUrl>
#include <QUrlQuery>

VersionDownloader::VersionDownloader(QObject* parent) : QObject(parent)
{
    connect(&_webCtrl, &QNetworkAccessManager::finished, this, &VersionDownloader::updateManifestDownloaded);
}

VersionDownloader::~VersionDownloader()
{
}

void VersionDownloader::performCheck()
{
    QString versionString = Util::currentVersion();
#ifdef DEBUG
    versionString += QString("-%1-%2").arg(GIT_BRANCH, GIT_COMMIT_HASH);
#endif

    QUrlQuery query;
    query.addQueryItem("v", versionString);
    query.addQueryItem("os", QSysInfo::kernelType());

    QUrl versionManifest("https://modbusscope.jensgeudens.workers.dev");
    versionManifest.setQuery(query);

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
