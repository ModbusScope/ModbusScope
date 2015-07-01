#include <QColor>
#include "guimodel.h"


const QString GuiModel::_cWindowTitle = QString("ModbusScope");

const QList<QColor> GuiModel::_colorlist = QList<QColor>() << QColor(0, 0, 0)
                                                           << QColor(0, 0, 255)
                                                           << QColor(0, 255, 255)
                                                           << QColor(0, 255, 0)
                                                           << QColor(220, 220, 0)
                                                           << QColor(220, 153, 14)
                                                           << QColor(255, 165, 0)
                                                           << QColor(255, 0, 0)
                                                           << QColor(255, 160, 122)
                                                           << QColor(230, 104, 86)
                                                           << QColor(205, 205, 180)
                                                           << QColor(157, 153, 120)
                                                           << QColor(139, 69, 19)
                                                           << QColor(255, 20, 147)
                                                           << QColor(74, 112, 139)
                                                           << QColor(46, 139, 87)
                                                           << QColor(128, 0, 128)
                                                           << QColor(189, 183, 107)
                                                           ;

GuiModel::GuiModel(QObject *parent) : QObject(parent)
{
    _graphData.clear();
    _frontGraph = 0;
    _projectFilePath = "";
    _dataFilePath = "";
    _bHighlightSamples = true;
    _bValueTooltip = false;
    _bLegendVisibility = true;
    _legendPosition = BasicGraphView::LEGEND_RIGHT;
    _communicationState = INIT;
    _windowTitle = _cWindowTitle;

    QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (docPath.size() > 0)
    {
        _lastDir = docPath[0];
    }

    _guiSettings.xScaleMode = BasicGraphView::SCALE_AUTO;
    _guiSettings.yScaleMode = BasicGraphView::SCALE_AUTO;
    _guiSettings.yMax = 10;
    _guiSettings.yMin = 0;
    _guiSettings.xslidingInterval = 30;    
}

GuiModel::~GuiModel()
{

}

void GuiModel::triggerUpdate(void)
{
    if (_graphData.size() > 0)
    {
        for (quint8 idx = 0; idx < _graphData.size(); idx++)
        {
            emit graphVisibilityChanged(idx);
        }
    }

    emit frontGraphChanged();
    emit highlightSamplesChanged();
    emit valueTooltipChanged();
    emit windowTitleChanged();

    emit graphCleared();
    emit legendVisibilityChanged();
    emit legendPositionChanged();

    emit yAxisMinMaxchanged();
    emit xAxisSlidingIntervalChanged();

    emit xAxisScalingChanged();
    emit yAxisScalingChanged();

    emit communicationStateChanged();
    emit projectFilePathChanged();
    emit dataFilePathChanged();
}

void GuiModel::addGraphs(QList<RegisterData> registerList)
{
    quint32 colorIndex = 0;

    for (qint32 idx = 0; idx < registerList.size(); idx++)
    {
        GraphData * pGraphData = new GraphData();

        pGraphData->bVisibility = true;
        pGraphData->label = registerList[idx].text();;

        if (registerList[idx].color().isValid())
        {
            pGraphData->color = registerList[idx].color();
        }
        else
        {
            colorIndex %= _colorlist.size();
            pGraphData->color = _colorlist[colorIndex];
            colorIndex++;
        }

        _graphData.append(pGraphData);
    }

    emit graphsAdded();
}

void GuiModel::addGraphs(QList<RegisterData> registerList, QList<double> timeData, QList<QList<double> > data)
{
    // TODO: time data
    addGraphs(registerList);

    emit graphsAddData(timeData, data);
}

void GuiModel::clearGraph()
{
    _graphData.clear();
    setFrontGraph(-1);
    emit graphCleared();
}

quint32 GuiModel::graphCount()
{
    if (_graphData.size() > 0)
    {
        return _graphData.size();
    }
    else
    {
        return 0;
    }
}

bool GuiModel::graphVisibility(quint32 index) const
{
    return _graphData[index]->bVisibility;
}

QColor GuiModel::graphColor(quint32 index) const
{
    return _graphData[index]->color;
}

QString GuiModel::graphLabel(quint32 index) const
{
    return _graphData[index]->label;
}

void GuiModel::setGraphVisibility(quint32 index, const bool &value)
{
    if (_graphData[index]->bVisibility != value)
    {
         _graphData[index]->bVisibility = value;
         emit graphVisibilityChanged(index);
    }
}

qint32 GuiModel::frontGraph() const
{
    return _frontGraph;
}

void GuiModel::setFrontGraph(const qint32 &frontGraph)
{
    if (_frontGraph != frontGraph)
    {
        _frontGraph = frontGraph;

        if (frontGraph != -1)
        {
            emit frontGraphChanged();
        }
    }
}

bool GuiModel::highlightSamples() const
{
    return _bHighlightSamples;
}

void GuiModel::setHighlightSamples(bool bHighlightSamples)
{
    if (_bHighlightSamples != bHighlightSamples)
    {
        _bHighlightSamples = bHighlightSamples;
         emit highlightSamplesChanged();
    }
}

bool GuiModel::valueTooltip() const
{
    return _bValueTooltip;
}

void GuiModel::setValueTooltip(bool bValueTooltip)
{
    if (_bValueTooltip != bValueTooltip)
    {
        _bValueTooltip = bValueTooltip;
         emit valueTooltipChanged();
    }
}

QString GuiModel::windowTitle()
{
    return _windowTitle;
}

void GuiModel::setWindowTitleDetail(QString detail)
{
    QString tmpTitle;
    if (detail == "")
    {
        tmpTitle = _cWindowTitle;
    }
    else
    {
        tmpTitle = QString(tr("%1 - %2")).arg(_cWindowTitle).arg(detail);
    }

    if (tmpTitle != _windowTitle)
    {
        _windowTitle = tmpTitle;
        emit windowTitleChanged();
    }
}

bool GuiModel::legendVisibility()
{
    return _bLegendVisibility;
}

void GuiModel::setLegendVisibility(bool bLegendVisibility)
{
    if (_bLegendVisibility != bLegendVisibility)
    {
        _bLegendVisibility = bLegendVisibility;
         emit legendVisibilityChanged();
    }
}

BasicGraphView::LegendsPositionOptions GuiModel::legendPosition()
{
    return _legendPosition;
}

void GuiModel::setLegendPosition(BasicGraphView::LegendsPositionOptions pos)
{
    if (_legendPosition != pos)
    {
        _legendPosition = pos;
         emit legendPositionChanged();
    }
}

QString GuiModel::projectFilePath()
{
    return _projectFilePath;
}

QString GuiModel::dataFilePath()
{
    return _dataFilePath;
}

void GuiModel::setProjectFilePath(QString path)
{
    if (_projectFilePath != path)
    {
        _projectFilePath = path;
        emit projectFilePathChanged();
    }
}

void GuiModel::setDataFilePath(QString path)
{
    if (_dataFilePath != path)
    {
        _dataFilePath = path;
        emit dataFilePathChanged();
    }
}

void GuiModel::setLastDir(QString dir)
{
    _lastDir = dir;
}

QString GuiModel::lastDir()
{
    return _lastDir;
}

void GuiModel::setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    if (_guiSettings.xScaleMode != scaleMode)
    {
        _guiSettings.xScaleMode = scaleMode;
        emit xAxisScalingChanged();
    }
}

BasicGraphView::AxisScaleOptions GuiModel::xAxisScalingMode()
{
    return _guiSettings.xScaleMode;
}

void GuiModel::setxAxisSlidingInterval(qint32 slidingSec)
{
    if (_guiSettings.xslidingInterval != (quint32)slidingSec)
    {
        _guiSettings.xslidingInterval = (quint32)slidingSec;
        emit xAxisSlidingIntervalChanged();
    }

}

quint32 GuiModel::xAxisSlidingSec()
{
    return _guiSettings.xslidingInterval;
}

BasicGraphView::AxisScaleOptions GuiModel::yAxisScalingMode()
{
    return _guiSettings.yScaleMode;
}

void GuiModel::setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    if (_guiSettings.yScaleMode != scaleMode)
    {
        _guiSettings.yScaleMode = scaleMode;
        emit yAxisScalingChanged();
    }
}

void GuiModel::setyAxisMin(qint32 newMin)
{
    const qint32 diff = _guiSettings.yMax - _guiSettings.yMin;
    qint32 newMax = _guiSettings.yMax;

    if (newMin >= _guiSettings.yMax)
    {
        newMax = newMin + diff;
    }

    if (_guiSettings.yMin != newMin)
    {
        _guiSettings.yMin = newMin;
        setyAxisMax(newMax);
        emit yAxisMinMaxchanged();
    }
}

void GuiModel::setyAxisMax(qint32 newMax)
{
    const qint32 diff = _guiSettings.yMax - _guiSettings.yMin;

    qint32 newMin = _guiSettings.yMin;

    if (newMax <= _guiSettings.yMin)
    {
        newMin = newMax - diff;
    }

    if (_guiSettings.yMax != newMax)
    {
        _guiSettings.yMax = newMax;
        setyAxisMin(newMin);
        emit yAxisMinMaxchanged();
    }
}

void GuiModel::setCommunicationState(quint32 state)
{
    if (_communicationState != state)
    {
        _communicationState = state;
        emit communicationStateChanged();
    }
}

quint32 GuiModel::communicationState()
{
    return _communicationState;
}

qint32 GuiModel::yAxisMin()
{
    return _guiSettings.yMin;
}

qint32 GuiModel::yAxisMax()
{
    return _guiSettings.yMax;
}
