
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

    addTextNode(ProjectFileDefinitions::cAddressTag, QString("%1").arg(_pGraphDataModel->registerAddress(idx)), &registerElement);
    addTextNode(ProjectFileDefinitions::cTextTag, _pGraphDataModel->label(idx).toHtmlEscaped(), &registerElement);
    addTextNode(ProjectFileDefinitions::cUnsignedTag, convertBoolToText(_pGraphDataModel->isUnsigned(idx)), &registerElement);
    addTextNode(ProjectFileDefinitions::cMultiplyTag, Util::formatDoubleForExport(_pGraphDataModel->multiplyFactor(idx)), &registerElement);
    addTextNode(ProjectFileDefinitions::cDivideTag, Util::formatDoubleForExport(_pGraphDataModel->divideFactor(idx)), &registerElement);
    addTextNode(ProjectFileDefinitions::cColorTag, _pGraphDataModel->color(idx).name(), &registerElement);
    addTextNode(ProjectFileDefinitions::cBitmaskTag, QString("0x%1").arg(_pGraphDataModel->bitmask(idx), 0, 16), &registerElement);
    addTextNode(ProjectFileDefinitions::cShiftTag, QString("%1").arg(_pGraphDataModel->shift(idx)), &registerElement);
    addTextNode(ProjectFileDefinitions::cConnectionIdTag, QString("%1").arg(_pGraphDataModel->connectionId(idx)), &registerElement);

    pParentElement->appendChild(registerElement);
}

void ProjectFileExporter::createViewTag(QDomElement * pParentElement)
{
    QDomElement viewElement = _domDocument.createElement(ProjectFileDefinitions::cViewTag);
    QDomElement scaleElement = _domDocument.createElement(ProjectFileDefinitions::cScaleTag);

    /* Create xAxisElement tag */
    QDomElement xAxisElement = _domDocument.createElement(ProjectFileDefinitions::cXaxisTag);

    if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_SLIDING)
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


    if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_WINDOW_AUTO)
    {
        yAxisElement.setAttribute(ProjectFileDefinitions::cModeAttribute, ProjectFileDefinitions::cWindowAutoValue);
    }
    else if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MINMAX)
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




