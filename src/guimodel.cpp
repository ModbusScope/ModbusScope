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
    _loadedFile = "";
    _bHighlightSamples = true;
    _bValueTooltip = false;
    _bLegendVisibility = true;

    _guiSettings.scaleXSetting = BasicGraphView::SCALE_AUTO;
    _guiSettings.scaleYSetting = BasicGraphView::SCALE_AUTO;
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
}

void GuiModel::addGraphs(QList<RegisterData> registerList)
{
    quint32 colorIndex = 0;

    for (qint32 idx = 1; idx < registerList.size(); idx++)
    {
        GraphData * pGraphData = new GraphData();

        pGraphData->bVisibility = true;
        pGraphData->label = registerList[idx].getText();;

        if (registerList[i].getColor().isValid())
        {
            pGraphData->color = registerList[i].getColor();
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

void GuiModel::addGraphs(QList<RegisterData> registerList, QList<QList<double> > data)
{
    addGraphs(registerList);

    emit graphsAddedWithData(data);
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

QString GuiModel::loadedFile() const
{
    return _loadedFile;
}

void GuiModel::setLoadedFile(const QString &loadedFile)
{
    if (_loadedFile != loadedFile)
    {
        _loadedFile = loadedFile;
         emit loadedFileChanged();
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

QColor GuiModel::getColor(const quint32 index)
{
    const quint32 colorIndex = index % _colorlist.size();
    return _colorlist[colorIndex];
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

QString GuiModel::projectFilePath()
{
    return _projectFilePath;
}

QString GuiModel::lastDataFilePath()
{
    return _lastDataFilePath;
}

void GuiModel::setProjectFilePath(QString path)
{
    _projectFilePath = path;
}

void GuiModel::setLastDataFilePath(QString path)
{
    _lastDataFilePath = path;
}

void GuiModel::setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    if (_guiSettings.scaleXSetting != scaleMode)
    {
        _guiSettings.scaleXSetting = scaleMode;
        emit xAxisScalingChanged();
    }
}

BasicGraphView::AxisScaleOptions GuiModel::xAxisScalingMode()
{
    return _guiSettings.xScaleMode;
}

void GuiModel::setxAxisSlidingInterval(quint32 slidingSec)
{
    if (_guiSettings.scaleXSetting != slidingSec * 1000)
    {
        _guiSettings.xslidingInterval = slidingSec * 1000;
        emit xAxisSlidingIntervalChanged();
    }

}

quint32 GuiModel::xAxisSlidingSec()
{
    return _guiSettings.xslidingInterval * 1000;
}

BasicGraphView::AxisScaleOptions GuiModel::yAxisScalingMode()
{
    return _guiSettings.yScaleMode;
}

void GuiModel::setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    if (_guiSettings.scaleYSetting != scaleMode)
    {
        _guiSettings.scaleYSetting = scaleMode;
        emit yAxisScalingChanged();
    }
}

void GuiModel::setyAxisMin(quint32 min)
{
    const qint32 diff = GuiSettings.yMax - GuiSettings.yMin;
    qint32 newMax = GuiSettings.yMax;

    if (newMin >= GuiSettings.yMax)
    {
        newMax = newMin + diff;
    }

    if (_guiSettings.yMin != min)
    {
        _guiSettings.yMin = min;
        setyAxisMax(newMax);
        emit yAxisMinMaxchanged();
    }
}

void GuiModel::setyAxisMax(quint32 max)
{
    const qint32 diff = GuiSettings.yMax - GuiSettings.yMin;

    qint32 newMin = GuiSettings.yMin;

    if (newMax <= GuiSettings.yMin)
    {
        newMin = newMax - diff;
    }

    if (_guiSettings.yMax != max)
    {
        _guiSettings.yMax = max;
        setyAxisMin(newMin);
        emit yAxisMaxchanged();
    }
}

qint32 GuiModel::yAxisMin()
{
    return _guiSettings.yMin;
}

qint32 GuiModel::yAxisMax()
{
    return _guiSettings.yMax;
}

void GuiModel::xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{
    todo
    QCPRange range = newRange;

    if (newRange.upper <= 0)
    {
        range.upper = oldRange.upper;
    }

    if (newRange.lower <= 0)
    {
        range.lower = 0;
    }

    _pPlot->xAxis->setRange(range);
}
