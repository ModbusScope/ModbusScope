
#include <QDesktopServices>

#include "util.h"
#include "updatenotify.h"

UpdateNotify::UpdateNotify(QObject *parent) :
    QObject(parent)
{
    connect(&_versionDownloader, SIGNAL(versionDownloaded()), this, SLOT (handleVersionData()));
}

void UpdateNotify::checkForUpdate()
{
    _versionDownloader.performCheck();
}

void UpdateNotify::handleVersionData()
{
    QStringList updateFields = QString(_versionDownloader.downloadedVersionData()).trimmed().split(',');

    // Parse data
    bool bRet = true;
    if (updateFields.size() >= 3)
    {
        _version = updateFields[0];
        _dataLevel = updateFields[1].toUInt(&bRet);
        _link =  updateFields[2];
    }

    if (bRet)
    {
        _bValidData = true;

        const UpdateState versionState = checkVersions(Util::currentVersion(), _version);
        const bool bDataLevelUpdate = (Util::currentDataLevel() != _dataLevel);

        // Emit signal only on success
        updateCheckResult(versionState, bDataLevelUpdate);

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

    if (
        (currentVersionFields[0] < latestVersionFields[0])
        || (currentVersionFields[1] < latestVersionFields[1])
    )
    {
        return MINOR_MAJOR_UPDATE;
    }
    else if (currentVersionFields[2] < latestVersionFields[2])
    {
        return REV_UPDATE;
    }
    else
    {
        return LATEST;
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

quint32 UpdateNotify::dataLevel() const
{
    return _dataLevel;
}
