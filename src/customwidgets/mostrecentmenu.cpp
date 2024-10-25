#include "mostrecentmenu.h"

#include "recentfilemodule.h"
#include <QAction>

MostRecentMenu::MostRecentMenu(QMenu* mostRecentMenu, RecentFileModule *recentFileModule, QObject *parent)
    : QObject{parent}, _mostRecentMenu(mostRecentMenu), _recentFileModule(recentFileModule)
{
    connect(_recentFileModule, &RecentFileModule::mostRecentProjectFileUpdated, this, &MostRecentMenu::handleMostRecentProjectFileUpdate);

    handleMostRecentProjectFileUpdate();
}

void MostRecentMenu::handleMostRecentProjectFileUpdate()
{
    _recentProjectFiles = _recentFileModule->recentProjectFiles();

    _mostRecentMenu->clear();

    if (_recentProjectFiles.size() <= 0)
    {
        auto noRecent = new QAction("No recent project files");

        noRecent->setEnabled(false);
        _mostRecentMenu->addAction(noRecent);
    }
    else
    {
        for (quint32 idx = 0; idx < _recentProjectFiles.size(); idx++)
        {
            auto projectFileAction = new QAction(_recentProjectFiles[idx]);
            projectFileAction->setData(idx);

            connect(projectFileAction, &QAction::triggered, this, &MostRecentMenu::handleRecentProjectFileClicked);

            _mostRecentMenu->addAction(projectFileAction);
        }
    }
}

void MostRecentMenu::handleRecentProjectFileClicked()
{
    QAction* const pAction = qobject_cast<QAction * const>(QObject::sender());
    QString filename = _recentProjectFiles.at(pAction->data().toInt());

    emit openRecentProject(filename);
}
