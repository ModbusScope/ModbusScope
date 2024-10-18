#ifndef RECENTFILEMODULE_H
#define RECENTFILEMODULE_H

#include <QSettings>
#include <QList>

class RecentFileModule : public QObject
{
    Q_OBJECT
public:
    explicit RecentFileModule(QObject *parent = nullptr);

    void clearRecentProjectFiles();
    void loadRecentProjectFiles();
    void updateRecentProjectFiles(const QString filePath);

    QList<QString> recentProjectFiles();

signals:
    void mostRecentProjectFileUpdated();

private:
    QSettings _settings;
    QList<QString> _recentProjectFiles;

    static const quint32 _cMostRecentCount = 5;
    static const QString _cProjectFileSection;
};

#endif // RECENTFILEMODULE_H
