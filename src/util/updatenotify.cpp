
#include <QRegularExpression>

#include "util.h"
#include "updatenotify.h"
#include "versiondownloader.h"

UpdateNotify::UpdateNotify(VersionDownloader* pVersionDownloader, QObject *parent) :
    QObject(parent)
{
    _bValidData = false;
    _pVersionDownloader = pVersionDownloader;

    connect(_pVersionDownloader, SIGNAL(versionDownloaded()), this, SLOT (handleVersionData()));
}

void UpdateNotify::checkForUpdate()
{
    _pVersionDownloader->performCheck();
}

void UpdateNotify::handleVersionData()
{
    QRegularExpression dateParseRegex;
    dateParseRegex.setPattern("(\\d+\\.\\d+\\.\\d+)");

    QRegularExpressionMatch match = dateParseRegex.match(_pVersionDownloader->version());

    _version = match.captured(1);
    _link = _pVersionDownloader->url();

    if (!_version.isEmpty() && !_link.isEmpty())
    {
        _bValidData = true;

        const UpdateState versionState = checkVersions(Util::currentVersion(), _version);

        updateCheckResult(versionState);
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

QString UpdateNotify::version() const
{
    return _version;
}

QString UpdateNotify::link() const
{
    return _link;
}
