
#include <QDomElement>
#include <QTextStream>

#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"

#include "projectfiledefinitions.h"
#include "projectfileexporter.h"


ProjectFileExporter::ProjectFileExporter(GuiModel *pGuiModel, SettingsModel *pSettingsModel, GraphDataModel * pGraphDataModel, QObject *parent) : QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pSettingsModel = pSettingsModel;
    _pGraphDataModel = pGraphDataModel;
}

void ProjectFileExporter::exportProjectFile(QString projectFile)
{
    createDomDocument();

    QFile file(projectFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {

        //QString::toHtmlEscaped() => zorgt voor conversie van speciale tekens
        QTextStream stream(&file);

        qDebug() << _domDocument.toString();
        stream << _domDocument.toString();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope settings export error"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Export settings to file (%1) failed").arg(filePath));
        msgBox.exec();
    }

    /* TODO
    Export settings moet ge-enabled worden als configuratie veranderd wordt
    Icon voor export settings
    */
}


void ProjectFileExporter::createDomDocument()
{
    _domDocument.clear();

    QDomElement rootElement = _domDocument.createElement(ProjectFileDefinitions::cModbusScopeTag);

    _domDocument.appendChild(rootElement);


}
