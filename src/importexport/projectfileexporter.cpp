
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

    qDebug() << _domDocument.toString();

    // TODO
#if 0
    QFile file(projectFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {

        //QString::toHtmlEscaped() => zorgt voor conversie van speciale tekens
        QTextStream stream(&file);
        stream << _domDocument.toString();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("ModbusScope settings export error"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Export settings to file (%1) failed").arg(projectFile));
        msgBox.exec();
    }
#endif

    /* TODO
    Export settings moet ge-enabled worden als configuratie veranderd wordt
    Icon voor export settings
    */
}


void ProjectFileExporter::createDomDocument()
{
    _domDocument.clear();

    QDomElement rootElement = _domDocument.createElement(ProjectFileDefinitions::cModbusScopeTag);
    rootElement.setAttribute(ProjectFileDefinitions::cDatalevelAttribute, QString("%1").arg(ProjectFileDefinitions::cCurrentDataLevel));

    createModbusTag(&rootElement);

    _domDocument.appendChild(rootElement);

}

void ProjectFileExporter::createModbusTag(QDomElement * pParentElement)
{
    QDomElement modbusElement = _domDocument.createElement(ProjectFileDefinitions::cModbusTag);

    createConnectionTag(&modbusElement);
    createLogTag(&modbusElement);

    pParentElement->appendChild(modbusElement);
}

void ProjectFileExporter::createConnectionTag(QDomElement * pParentElement)
{
    QDomElement connectionElement = _domDocument.createElement(ProjectFileDefinitions::cConnectionTag);

    addTextNode(ProjectFileDefinitions::cIpTag, _pSettingsModel->ipAddress(), &connectionElement);
    addTextNode(ProjectFileDefinitions::cPortTag, QString("%1").arg(_pSettingsModel->port()), &connectionElement);
    addTextNode(ProjectFileDefinitions::cSlaveIdTag, QString("%1").arg(_pSettingsModel->slaveId()), &connectionElement);
    addTextNode(ProjectFileDefinitions::cTimeoutTag, QString("%1").arg(_pSettingsModel->timeout()), &connectionElement);
    addTextNode(ProjectFileDefinitions::cConsecutiveMaxTag, QString("%1").arg(_pSettingsModel->consecutiveMax()), &connectionElement);

    pParentElement->appendChild(connectionElement);
}

void ProjectFileExporter::createLogTag(QDomElement * pParentElement)
{
    QDomElement logElement = _domDocument.createElement(ProjectFileDefinitions::cLogTag);

    addTextNode(ProjectFileDefinitions::cPollTimeTag, QString("%").arg(_pSettingsModel->pollTime()), &logElement);
    addTextNode(ProjectFileDefinitions::cAbsoluteTimesTag, convertBoolToText(_pSettingsModel->absoluteTimes()), &logElement);

    /* Create logtofile tag */
    QDomElement logToFileElement = _domDocument.createElement(ProjectFileDefinitions::cLogToFileTag);
    logToFileElement.setAttribute(ProjectFileDefinitions::cEnabledAttribute, convertBoolToText(_pSettingsModel->writeDuringLog()));
    if (
            _pSettingsModel->writeDuringLog()
            && (_pSettingsModel->writeDuringLogFile() != SettingsModel::defaultLogPath())
            )
    {
        // TODO: check if path is exported correctly on Windows
        addTextNode(ProjectFileDefinitions::cFilenameTag, _pSettingsModel->writeDuringLogFile(), &logToFileElement);
    }
    logElement.appendChild(logToFileElement);

    pParentElement->appendChild(logElement);
}

QString ProjectFileExporter::convertBoolToText(bool bValue)
{
    QString boolText = bValue ? tr("true") : tr("false");
    return boolText;
}

void ProjectFileExporter::addTextNode(QString tagName, QString tagValue, QDomElement * pParentElement)
{
    QDomElement tag = _domDocument.createElement(tagName);
    QDomText valueNode = _domDocument.createTextNode(tagValue);
    tag.appendChild(valueNode);

    pParentElement->appendChild(tag);
}


