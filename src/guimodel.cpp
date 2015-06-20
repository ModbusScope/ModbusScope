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

void ScopeGui::setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    _settings.scaleXSetting = scaleMode;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}

void ScopeGui::setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode, quint32 interval)
{
    _settings.scaleXSetting = scaleMode;
    _settings.xslidingInterval = interval * 1000;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}

void ScopeGui::setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode)
{
    _settings.scaleYSetting = scaleMode;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}


void ScopeGui::setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode, qint32 min, qint32 max)
{
    _settings.scaleYSetting = scaleMode;
    _settings.yMin = min;
    _settings.yMax = max;

    if (scaleMode != SCALE_MANUAL)
    {
        scalePlot();
    }
}


void ScopeGui::setxAxisSlidingInterval(int interval)
{
    _settings.xslidingInterval = (quint32)interval * 1000;
    updateXScalingUi(ScopeGui::SCALE_SLIDING); // set sliding window scaling
}

void ScopeGui::setyAxisMinMax(quint32 min, quint32 max)
{
    _settings.yMin = min;
    _settings.yMax = max;
    updateYScalingUi(ScopeGui::SCALE_MINMAX); // set min max scaling
}

qint32 ScopeGui::getyAxisMin()
{
    return _settings.yMin;
}

qint32 ScopeGui::getyAxisMax()
{
    return _settings.yMax;
}

void ScopeGui::xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{
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
