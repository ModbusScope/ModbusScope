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
    void createModbusTag(QDomElement * pParentElement);

    void createConnectionTags(QDomElement * pParentElement);
    void createLogTag(QDomElement * pParentElement);

    void createScopeTag(QDomElement * pParentElement);
    void createRegisterTag(QDomElement * pParentElement, qint32 idx);

    void createViewTag(QDomElement * pParentElement);

    QString convertBoolToText(bool bValue);
    void addTextNode(QString tagName, QString tagValue, QDomElement * pParentElement);

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;

    QDomDocument _domDocument;

};

#endif // PROJECTFILEEXPORTER_H
