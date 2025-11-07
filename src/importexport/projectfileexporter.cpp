
#include <QDomElement>
#include <QTextStream>

#include "importexport/projectfiledefinitions.h"
#include "importexport/projectfileexporter.h"
#include "models/connectiontypes.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/settingsmodel.h"
#include "util/util.h"

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
    createDeviceTags(&modbusElement);
    createLogTag(&modbusElement);

    pParentElement->appendChild(modbusElement);
}

void ProjectFileExporter::createConnectionTags(QDomElement * pParentElement)
{
    for (quint8 i = 0u; i < ConnectionTypes::ID_CNT; i++)
    {
        auto connData = _pSettingsModel->connectionSettings(i);
        QDomElement connectionElement = _domDocument.createElement(ProjectFileDefinitions::cConnectionTag);

        addTextNode(ProjectFileDefinitions::cConnectionEnabledTag, convertBoolToText(_pSettingsModel->connectionState(i)), &connectionElement);

        addTextNode(ProjectFileDefinitions::cConnectionIdTag, QString("%1").arg(i), &connectionElement);

        if (connData->connectionType() == ConnectionTypes::TYPE_TCP)
        {
            addTextNode(ProjectFileDefinitions::cConnectionTypeTag, QString("tcp"), &connectionElement);
        }
        else
        {
            addTextNode(ProjectFileDefinitions::cConnectionTypeTag, QString("serial"), &connectionElement);
        }

        addTextNode(ProjectFileDefinitions::cIpTag, connData->ipAddress(), &connectionElement);
        addTextNode(ProjectFileDefinitions::cPortTag, QString("%1").arg(connData->port()), &connectionElement);

        addTextNode(ProjectFileDefinitions::cPortNameTag, QString("%1").arg(connData->portName()), &connectionElement);
        addTextNode(ProjectFileDefinitions::cBaudrateTag, QString("%1").arg(connData->baudrate()), &connectionElement);

        addTextNode(ProjectFileDefinitions::cParityTag, QString("%1").arg(connData->parity()), &connectionElement);
        addTextNode(ProjectFileDefinitions::cStopBitsTag, QString("%1").arg(connData->stopbits()), &connectionElement);
        addTextNode(ProjectFileDefinitions::cDataBitsTag, QString("%1").arg(connData->databits()), &connectionElement);
        addTextNode(ProjectFileDefinitions::cTimeoutTag, QString("%1").arg(connData->timeout()), &connectionElement);
        addTextNode(ProjectFileDefinitions::cPersistentConnectionTag,
                    convertBoolToText(connData->persistentConnection()), &connectionElement);

        pParentElement->appendChild(connectionElement);
    }
}

void ProjectFileExporter::createDeviceTags(QDomElement* pParentElement)
{
    QList<deviceId_t> deviceList = _pSettingsModel->deviceList();

    for (const deviceId_t& deviceId : deviceList)
    {
        Device* device = _pSettingsModel->deviceSettings(deviceId);
        QDomElement deviceElement = _domDocument.createElement(ProjectFileDefinitions::cDeviceTag);

        addTextNode(ProjectFileDefinitions::cDeviceIdTag, QString("%1").arg(deviceId), &deviceElement);
        addTextNode(ProjectFileDefinitions::cDeviceNameTag, device->name(), &deviceElement);
        addTextNode(ProjectFileDefinitions::cConnectionIdTag, QString("%1").arg(device->connectionId()),
                    &deviceElement);
        addTextNode(ProjectFileDefinitions::cSlaveIdTag, QString("%1").arg(device->slaveId()), &deviceElement);
        addTextNode(ProjectFileDefinitions::cConsecutiveMaxTag, QString("%1").arg(device->consecutiveMax()),
                    &deviceElement);
        addTextNode(ProjectFileDefinitions::cInt32LittleEndianTag, convertBoolToText(device->int32LittleEndian()),
                    &deviceElement);

        pParentElement->appendChild(deviceElement);
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
    addTextNode(ProjectFileDefinitions::cValueAxisTag, QString("%1").arg(_pGraphDataModel->valueAxis(idx)), &registerElement);

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

    yAxisElement.setAttribute(ProjectFileDefinitions::cAxisAttribute, 0);

    if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cWindowAutoValue);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cMinmaxValue);

        addTextNode(ProjectFileDefinitions::cMinTag, QString("%1").arg(Util::formatDoubleForExport(_pGuiModel->yAxisMin())), &yAxisElement);
        addTextNode(ProjectFileDefinitions::cMaxTag, QString("%1").arg(Util::formatDoubleForExport(_pGuiModel->yAxisMax())), &yAxisElement);
    }
    else
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cAutoValue);
    }
    scaleElement.appendChild(yAxisElement);

    /* Create yAxisElement tag */
    QDomElement y2AxisElement = _domDocument.createElement(ProjectFileDefinitions::cYaxisTag);

    y2AxisElement.setAttribute(ProjectFileDefinitions::cAxisAttribute, 1);

    if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        y2AxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cWindowAutoValue);
    }
    else if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        y2AxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cMinmaxValue);

        addTextNode(ProjectFileDefinitions::cMinTag, QString("%1").arg(Util::formatDoubleForExport(_pGuiModel->y2AxisMin())), &y2AxisElement);
        addTextNode(ProjectFileDefinitions::cMaxTag, QString("%1").arg(Util::formatDoubleForExport(_pGuiModel->y2AxisMax())), &y2AxisElement);
    }
    else
    {
        y2AxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cAutoValue);
    }
    scaleElement.appendChild(y2AxisElement);

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
