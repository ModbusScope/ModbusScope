
#include <QDomElement>
#include <QTextStream>

#include "guimodel.h"
#include "settingsmodel.h"
#include "graphdatamodel.h"
#include "util.h"

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
        QTextStream stream(&file);
        stream << _domDocument.toString();
    }
    else
    {
        Util::showError(tr("Export settings to file (%1) failed").arg(projectFile));
    }
}


void ProjectFileExporter::createDomDocument()
{
    _domDocument.clear();

    QDomNode versionNode(_domDocument.createProcessingInstruction("xml", "version=\"1.0\""));
    _domDocument.appendChild(versionNode);

    QDomElement rootElement = _domDocument.createElement(ProjectFileDefinitions::cModbusScopeTag);
    rootElement.setAttribute(ProjectFileDefinitions::cDatalevelAttribute, QString("%1").arg(ProjectFileDefinitions::cCurrentDataLevel));

    createModbusTag(&rootElement);
    createScopeTag(&rootElement);
    createViewTag(&rootElement);

    _domDocument.appendChild(rootElement);

}

void ProjectFileExporter::createModbusTag(QDomElement * pParentElement)
{
    QDomElement modbusElement = _domDocument.createElement(ProjectFileDefinitions::cModbusTag);

    createConnectionTags(&modbusElement);
    createLogTag(&modbusElement);

    pParentElement->appendChild(modbusElement);
}

void ProjectFileExporter::createConnectionTags(QDomElement * pParentElement)
{
    for (quint8 i = 0u; i < Connection::ID_CNT; i++)
    {
        QDomElement connectionElement = _domDocument.createElement(ProjectFileDefinitions::cConnectionTag);

        addTextNode(ProjectFileDefinitions::cConnectionEnabledTag, convertBoolToText(_pSettingsModel->connectionState(i)), &connectionElement);

        addTextNode(ProjectFileDefinitions::cConnectionIdTag, QString("%1").arg(i), &connectionElement);

        if (_pSettingsModel->connectionType(i) == Connection::TYPE_TCP)
        {
            addTextNode(ProjectFileDefinitions::cConnectionTypeTag, QString("tcp"), &connectionElement);
        }
        else
        {
            addTextNode(ProjectFileDefinitions::cConnectionTypeTag, QString("serial"), &connectionElement);
        }

        addTextNode(ProjectFileDefinitions::cIpTag, _pSettingsModel->ipAddress(i), &connectionElement);
        addTextNode(ProjectFileDefinitions::cPortTag, QString("%1").arg(_pSettingsModel->port(i)), &connectionElement);

        addTextNode(ProjectFileDefinitions::cPortNameTag, QString("%1").arg(_pSettingsModel->portName(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cBaudrateTag, QString("%1").arg(_pSettingsModel->baudrate(i)), &connectionElement);

        addTextNode(ProjectFileDefinitions::cParityTag, QString("%1").arg(_pSettingsModel->parity(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cStopBitsTag, QString("%1").arg(_pSettingsModel->stopbits(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cDataBitsTag, QString("%1").arg(_pSettingsModel->databits(i)), &connectionElement);

        addTextNode(ProjectFileDefinitions::cSlaveIdTag, QString("%1").arg(_pSettingsModel->slaveId(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cTimeoutTag, QString("%1").arg(_pSettingsModel->timeout(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cConsecutiveMaxTag, QString("%1").arg(_pSettingsModel->consecutiveMax(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cInt32LittleEndianTag, convertBoolToText(_pSettingsModel->int32LittleEndian(i)), &connectionElement);
        addTextNode(ProjectFileDefinitions::cPersistentConnectionTag, convertBoolToText(_pSettingsModel->persistentConnection(i)), &connectionElement);

        pParentElement->appendChild(connectionElement);
    }
}

void ProjectFileExporter::createLogTag(QDomElement * pParentElement)
{
    QDomElement logElement = _domDocument.createElement(ProjectFileDefinitions::cLogTag);

    addTextNode(ProjectFileDefinitions::cPollTimeTag, QString("%1").arg(_pSettingsModel->pollTime()), &logElement);
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

void ProjectFileExporter::createScopeTag(QDomElement * pParentElement)
{
    QDomElement scopeElement = _domDocument.createElement(ProjectFileDefinitions::cScopeTag);

    for (qint32 idx = 0; idx < _pGraphDataModel->size(); idx++)
    {
        createRegisterTag(&scopeElement, idx);
    }

    pParentElement->appendChild(scopeElement);
}

void ProjectFileExporter::createRegisterTag(QDomElement * pParentElement, qint32 idx)
{
    QDomElement registerElement = _domDocument.createElement(ProjectFileDefinitions::cRegisterTag);

    registerElement.setAttribute(ProjectFileDefinitions::cActiveAttribute, convertBoolToText(_pGraphDataModel->isActive(idx)));

    addTextNode(ProjectFileDefinitions::cTextTag, _pGraphDataModel->label(idx).toHtmlEscaped(), &registerElement);
    addCDataNode(ProjectFileDefinitions::cExpressionTag, _pGraphDataModel->expression(idx), &registerElement);
    addTextNode(ProjectFileDefinitions::cColorTag, _pGraphDataModel->color(idx).name(), &registerElement);

    pParentElement->appendChild(registerElement);
}

void ProjectFileExporter::createViewTag(QDomElement * pParentElement)
{
    QDomElement viewElement = _domDocument.createElement(ProjectFileDefinitions::cViewTag);
    QDomElement scaleElement = _domDocument.createElement(ProjectFileDefinitions::cScaleTag);

    /* Create xAxisElement tag */
    QDomElement xAxisElement = _domDocument.createElement(ProjectFileDefinitions::cXaxisTag);

    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_SLIDING)
    {
        xAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cSlidingValue);

        addTextNode(ProjectFileDefinitions::cSlidingintervalTag, QString("%1").arg(_pGuiModel->xAxisSlidingSec()), &xAxisElement);
    }
    else
    {
        xAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cAutoValue);
    }
    scaleElement.appendChild(xAxisElement);


    /* Create yAxisElement tag */
    QDomElement yAxisElement = _domDocument.createElement(ProjectFileDefinitions::cYaxisTag);


    if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cWindowAutoValue);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cMinmaxValue);

        addTextNode(ProjectFileDefinitions::cMinTag, QString("%1").arg(_pGuiModel->yAxisMin()), &yAxisElement);
        addTextNode(ProjectFileDefinitions::cMaxTag, QString("%1").arg(_pGuiModel->yAxisMax()), &yAxisElement);
    }
    else
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cAutoValue);
    }
    scaleElement.appendChild(yAxisElement);

    viewElement.appendChild(scaleElement);
    pParentElement->appendChild(viewElement);
}

QString ProjectFileExporter::convertBoolToText(bool bValue)
{
    QString boolText = bValue ? ProjectFileDefinitions::cTrueValue : ProjectFileDefinitions::cFalseValue;
    return boolText;
}

void ProjectFileExporter::addTextNode(QString tagName, QString tagValue, QDomElement * pParentElement)
{
    QDomElement tag = _domDocument.createElement(tagName);
    QDomText valueNode = _domDocument.createTextNode(tagValue);
    tag.appendChild(valueNode);

    pParentElement->appendChild(tag);
}

void ProjectFileExporter::addCDataNode(QString tagName, QString tagValue, QDomElement * pParentElement)
{
    QDomElement tag = _domDocument.createElement(tagName);
    QDomCDATASection valueNode = _domDocument.createCDATASection(tagValue);
    tag.appendChild(valueNode);

    pParentElement->appendChild(tag);
}
