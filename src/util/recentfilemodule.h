#ifndef RECENTFILEMODULE_H
#define RECENTFILEMODULE_H

#include <QList>
#include <QSettings>

class RecentFileModule : public QObject
{
    Q_OBJECT
public:
    explicit RecentFileModule(QObject* parent = nullptr);

    void clearRecentProjectFiles();
    void loadRecentProjectFiles();
    void updateRecentProjectFiles(const QString filePath);

    QList<QString> recentProjectFiles();

    static constexpr quint32 cMostRecentCount = 5;

signals:
    void mostRecentProjectFileUpdated();

private:
    QSettings _settings;
    QList<QString> _recentProjectFiles;

    static const QString _cProjectFileSection;
};

#endif // RECENTFILEMODULE_H
