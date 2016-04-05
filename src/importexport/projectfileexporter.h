#ifndef PROJECTFILEEXPORTER_H
#define PROJECTFILEEXPORTER_H

#include <QObject>
#include <QDomDocument>

#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"

class ProjectFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFileExporter(GuiModel *pGuiModel, SettingsModel *pSettingsModel, GraphDataModel * pGraphDataModel, QObject *parent = 0);

    void exportProjectFile(QString projectFile);

signals:

public slots:

private:

    void createDomDocument();

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;

    QDomDocument _domDocument;
};

#endif // PROJECTFILEEXPORTER_H
