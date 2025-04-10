
#include "updatenotify.h"

#include "util/versiondownloader.h"

#include <QRegularExpression>

UpdateNotify::UpdateNotify(VersionDownloader* pVersionDownloader, QString currentVersion, QObject *parent) :
    QObject(parent)
{
    _currentVersion = currentVersion;
    _bValidData = false;
    _versionState = VERSION_LATEST;
    _pVersionDownloader = pVersionDownloader;

    connect(_pVersionDownloader, &VersionDownloader::versionDownloaded, this, &UpdateNotify::handleVersionData);
}

void UpdateNotify::checkForUpdate()
{
    _pVersionDownloader->performCheck();
}

void UpdateNotify::handleVersionData()
{
    QRegularExpression dateParseRegex;
    dateParseRegex.setPattern(R"((\d+\.\d+\.\d+))");

    QRegularExpressionMatch match = dateParseRegex.match(_pVersionDownloader->version());

    _version = match.captured(1);
    _link = QUrl(_pVersionDownloader->url(), QUrl::StrictMode);

    QDate publishDate = QDate::fromString(_pVersionDownloader->publishDate(), Qt::ISODate);

    if (!_version.isEmpty() && _link.isValid() && publishDate.isValid())
    {
        _bValidData = true;

        _versionState = checkVersions(_currentVersion, _version);

        if (_versionState == VERSION_UPDATE_AVAILABLE)
        {
            _versionState = checkDate(publishDate);
        }

        emit updateCheckResult(_versionState);
    }
    else
    {
        _bValidData = false;
    }
}


UpdateNotify::UpdateState UpdateNotify::checkVersions(QString current, QString latest)
{
    QStringList currentVersionFields = current.split('.');
    QStringList latestVersionFields = latest.split('.');

    /* Check major */
    if (currentVersionFields[0] < latestVersionFields[0])
    {
        return VERSION_UPDATE_AVAILABLE;
    }
    else if (currentVersionFields[0] == latestVersionFields[0])
    {
        /* Major is equal, check minor */
        if (currentVersionFields[1] < latestVersionFields[1])
        {
            return VERSION_UPDATE_AVAILABLE;
        }
        else if (currentVersionFields[1] == latestVersionFields[1])
        {
            /* Minor is equal, check revision */
            if (currentVersionFields[2] < latestVersionFields[2])
            {
                return VERSION_UPDATE_AVAILABLE;
            }
            else
            {
                return VERSION_LATEST;
            }
        }
        else
        {
            return VERSION_LATEST;
        }
    }
    else
    {
        return VERSION_LATEST;
    }
}

UpdateNotify::UpdateState UpdateNotify::checkDate(QDate publishDate)
{
    if (publishDate.addDays(_cdaysDelay) < QDate::currentDate())
    {
        return VERSION_UPDATE_AVAILABLE;
    }
    else
    {
        return VERSION_LATEST;
    }
}

QString UpdateNotify::version() const
{
    return _version;
}

QUrl UpdateNotify::link() const
{
    return _link;
}

UpdateNotify::UpdateState UpdateNotify::versionState() const
{
    return _versionState;
}
