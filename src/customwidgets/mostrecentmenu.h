#ifndef MOSTRECENTMENU_H
#define MOSTRECENTMENU_H

#include <QMenu>

class RecentFileModule;

class MostRecentMenu : public QObject
{
    Q_OBJECT
public:
    explicit MostRecentMenu(QMenu* mostRecentMenu, RecentFileModule* recentFileModule, QObject *parent = nullptr);

signals:
    void openRecentProject(QString projectFile);

private slots:
    void handleMostRecentProjectFileUpdate();
    void handleRecentProjectFileClicked();

private:
    QMenu* _mostRecentMenu;
    RecentFileModule* _recentFileModule;

    QList<QString> _recentProjectFiles;

};

#endif // MOSTRECENTMENU_H
