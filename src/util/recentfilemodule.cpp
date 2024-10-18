#include "recentfilemodule.h"

const QString RecentFileModule::_cProjectFileSection = QString("recentProjectFiles");

RecentFileModule::RecentFileModule(QObject *parent)
    : QObject{parent}
{
    loadRecentProjectFiles();
}

void RecentFileModule::clearRecentProjectFiles()
{
    _recentProjectFiles.clear();
    _settings.remove(_cProjectFileSection);

    emit mostRecentProjectFileUpdated();
}

void RecentFileModule::loadRecentProjectFiles()
{
    int size = _settings.beginReadArray(_cProjectFileSection);
    for (int i = 0; i < size; ++i)
    {
        _settings.setArrayIndex(i);
        _recentProjectFiles.append(_settings.value("filePath").toString());
    }
    _settings.endArray();

    emit mostRecentProjectFileUpdated();
}

void RecentFileModule::updateRecentProjectFiles(const QString filePath)
{
    _recentProjectFiles.removeAll(filePath);
    _recentProjectFiles.prepend(filePath);

    while (_recentProjectFiles.size() > _cMostRecentCount)
    {
        _recentProjectFiles.removeLast();
    }

    _settings.beginWriteArray(_cProjectFileSection);
    for (int i = 0; i < _recentProjectFiles.size(); ++i)
    {
        _settings.setArrayIndex(i);
        _settings.setValue("filePath", _recentProjectFiles.at(i));
    }
    _settings.endArray();

    emit mostRecentProjectFileUpdated();
}

QList<QString> RecentFileModule::recentProjectFiles()
{
    return _recentProjectFiles;
}
