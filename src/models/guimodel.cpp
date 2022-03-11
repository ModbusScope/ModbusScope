#include <QColor>
#include <QStringList>
#include <QList>
#include <QStandardPaths>

#include "util.h"
#include "guimodel.h"


const QString GuiModel::_cWindowTitle = QString("ModbusScope");


const quint32 GuiModel::cDifferenceMask    = 1 << 0;
const quint32 GuiModel::cSlopeMask         = 1 << 1;
const quint32 GuiModel::cAverageMask       = 1 << 2;
const quint32 GuiModel::cMinimumMask       = 1 << 3;
const quint32 GuiModel::cMaximumMask       = 1 << 4;

const QStringList GuiModel::cMarkerExpressionStrings = QStringList()
                                                        <<  "Diff: %0\n"
                                                        <<  "Slope: %0\n"
                                                        <<  "Avg: %0\n"
                                                        <<  "Min: %0\n"
                                                        <<  "Max: %0\n";

const QList<quint32> GuiModel::cMarkerExpressionBits = QList<quint32>()
                        << GuiModel::cDifferenceMask
                        << GuiModel::cSlopeMask
                        << GuiModel::cAverageMask
                        << GuiModel::cMinimumMask
                        << GuiModel::cMaximumMask
                        ;
const QString GuiModel::cMarkerExpressionStart = QString("y1: %0\n");
const QString GuiModel::cMarkerExpressionEnd = QString("y2: %0\n");

GuiModel::GuiModel(QObject *parent) : QObject(parent)
{
    _frontGraph = 0;
    _projectFilePath = "";
    _bHighlightSamples = true;
    _bCursorValues = false;
    _guiState = INIT;
    _windowTitle = _cWindowTitle;

    _startTime = 0;
    _endTime = 0;
    _successCount = 0;
    _errorCount = 0;

    _zoomState = ZOOM_IDLE;

    QStringList docPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (docPath.size() > 0)
    {
        _lastDir = docPath[0];
    }
    _lastMbcImportedFile = QString();

    _guiSettings.xScaleMode = AxisMode::SCALE_AUTO;
    _guiSettings.yScaleMode = AxisMode::SCALE_AUTO;
    _guiSettings.y2ScaleMode = AxisMode::SCALE_AUTO;
    _guiSettings.yMax = 10;
    _guiSettings.y2Max = 10;
    _guiSettings.yMin = 0;
    _guiSettings.y2Min = 0;
    _guiSettings.xslidingInterval = 30;

    _bStartMarkerState = false;
    _startMarkerPos = 0;

    _bEndMarkerState = false;
    _endMarkerPos = 0;

    _bMarkerState = false;

    _markerExpressionMask = cDifferenceMask;
}

GuiModel::~GuiModel()
{

}

void GuiModel::triggerUpdate(void)
{
    if (_frontGraph != -1)
    {
        emit frontGraphChanged();
    }
    emit highlightSamplesChanged();
    emit cursorValuesChanged();
    emit windowTitleChanged();
    emit communicationStatsChanged();
    emit yAxisMinMaxchanged();
    emit y2AxisMinMaxchanged();
    emit xAxisSlidingIntervalChanged();
    emit xAxisScalingChanged();
    emit yAxisScalingChanged();
    emit y2AxisScalingChanged();
    emit guiStateChanged();
    emit projectFilePathChanged();

    emit markerStateChanged();
    emit markerExpressionMaskChanged();
}

/*
 Return index of activeGraphList */
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

bool GuiModel::cursorValues() const
{
    return _bCursorValues;
}

void GuiModel::setCursorValues(bool bCursorValues)
{
    if (_bCursorValues != bCursorValues)
    {
        _bCursorValues = bCursorValues;
         emit cursorValuesChanged();
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
        tmpTitle = QString(tr("%1 - %2")).arg(_cWindowTitle, detail);
    }

    if (tmpTitle != _windowTitle)
    {
        _windowTitle = tmpTitle;
        emit windowTitleChanged();
    }
}

QString GuiModel::projectFilePath()
{
    return _projectFilePath;
}

void GuiModel::setProjectFilePath(QString path)
{
    if (_projectFilePath != path)
    {
        _projectFilePath = path;
        emit projectFilePathChanged();
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

void GuiModel::setLastMbcImportedFile(QString file)
{
    _lastMbcImportedFile = file;
}

QString GuiModel::lastMbcImportedFile()
{
    return _lastMbcImportedFile;
}

void GuiModel::setxAxisScale(AxisMode::AxisScaleOptions scaleMode)
{
    if (_guiSettings.xScaleMode != scaleMode)
    {
        _guiSettings.xScaleMode = scaleMode;
        emit xAxisScalingChanged();
    }
}

AxisMode::AxisScaleOptions GuiModel::xAxisScalingMode()
{
    return _guiSettings.xScaleMode;
}

void GuiModel::setxAxisSlidingInterval(int slidingSec)
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

AxisMode::AxisScaleOptions GuiModel::yAxisScalingMode()
{
    return _guiSettings.yScaleMode;
}

AxisMode::AxisScaleOptions GuiModel::y2AxisScalingMode()
{
    return _guiSettings.y2ScaleMode;
}

void GuiModel::setyAxisScale(AxisMode::AxisScaleOptions scaleMode)
{
    if (_guiSettings.yScaleMode != scaleMode)
    {
        _guiSettings.yScaleMode = scaleMode;
        emit yAxisScalingChanged();
    }
}

void GuiModel::sety2AxisScale(AxisMode::AxisScaleOptions scaleMode)
{
    if (_guiSettings.y2ScaleMode != scaleMode)
    {
        _guiSettings.y2ScaleMode = scaleMode;
        emit y2AxisScalingChanged();
    }
}

void GuiModel::setyAxisMin(double newMin)
{
    const double diff = _guiSettings.yMax - _guiSettings.yMin;
    double newMax = _guiSettings.yMax;

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

void GuiModel::sety2AxisMin(double newMin)
{
    const double diff = _guiSettings.y2Max - _guiSettings.y2Min;
    double newMax = _guiSettings.y2Max;

    if (newMin >= _guiSettings.y2Max)
    {
        newMax = newMin + diff;
    }

    if (_guiSettings.y2Min != newMin)
    {
        _guiSettings.y2Min = newMin;
        sety2AxisMax(newMax);
        emit y2AxisMinMaxchanged();
    }
}

void GuiModel::setyAxisMax(double newMax)
{
    const double diff = _guiSettings.yMax - _guiSettings.yMin;

    double newMin = _guiSettings.yMin;

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

void GuiModel::sety2AxisMax(double newMax)
{
    const double diff = _guiSettings.y2Max - _guiSettings.y2Min;

    double newMin = _guiSettings.y2Min;

    if (newMax <= _guiSettings.y2Min)
    {
        newMin = newMax - diff;
    }

    if (_guiSettings.y2Max != newMax)
    {
        _guiSettings.y2Max = newMax;
        sety2AxisMin(newMin);
        emit y2AxisMinMaxchanged();
    }
}

void GuiModel::setGuiState(quint32 state)
{
    // GUI State is an exception, always send signal to make sure GUI is updated
    _guiState = state;
    emit guiStateChanged();
}

quint32 GuiModel::guiState()
{
    return _guiState;
}

qint64 GuiModel::communicationStartTime()
{
    return _startTime;
}

void GuiModel::setCommunicationStartTime(qint64 startTime)
{
    if (_startTime != startTime)
    {
        _startTime = startTime;
        // No signal yet
    }
}

qint64 GuiModel::communicationEndTime()
{
    return _endTime;
}

void GuiModel::setCommunicationEndTime(qint64 endTime)
{
    if (_endTime != endTime)
    {
        _endTime = endTime;
        // No signal yet
    }
}

quint32 GuiModel::communicationErrorCount()
{
    return _errorCount;
}

quint32 GuiModel::communicationSuccessCount()
{
    return _successCount;
}

double GuiModel::startMarkerPos()
{
    return _startMarkerPos;
}

double GuiModel::endMarkerPos()
{
    return _endMarkerPos;
}

bool GuiModel::markerState()
{
    return _bMarkerState;
}

quint32 GuiModel::markerExpressionMask()
{
    return _markerExpressionMask;
}

void GuiModel::setMarkerExpressionMask(quint32 mask)
{
    if (_markerExpressionMask != mask)
    {
        _markerExpressionMask = mask;

        emit markerExpressionMaskChanged();
    }
}

void GuiModel::incrementCommunicationStats(quint32 successes, quint32 errors)
{
    setCommunicationStats(communicationSuccessCount() + successes, communicationErrorCount() + errors);
}

GuiModel::ZoomState GuiModel::zoomState(void)
{
    return _zoomState;
}

void GuiModel::setZoomState(ZoomState zoomState)
{
    if (zoomState != _zoomState)
    {
        _zoomState = zoomState;

        emit zoomStateChanged();
    }
}

void GuiModel::setCommunicationStats(quint32 successCount, quint32 errorCount)
{
    if (
        (_successCount != successCount)
        || (_errorCount != errorCount)
        )
    {
        _successCount = successCount;
        _errorCount = errorCount;
        emit communicationStatsChanged();
    }
}

void GuiModel::clearMarkersState(void)
{
    setStartMarkerState(false);
    setEndMarkerState(false);
}

void GuiModel::setStartMarkerPos(double pos)
{
    if (
            (_startMarkerPos != pos)
            || (!_bStartMarkerState)
        )
    {
        if (
            (!_bEndMarkerState)
            || (pos != _endMarkerPos)
        )
        {
            setStartMarkerState(true);
            _startMarkerPos = pos;

            emit startMarkerPosChanged();
        }
    }
}

void GuiModel::setEndMarkerPos(double pos)
{
    if (
            (_endMarkerPos != pos)
            || (!_bEndMarkerState)
        )
    {
        if (
            (!_bStartMarkerState)
            || (pos != _startMarkerPos)
        )
        {
            setEndMarkerState(true);
            _endMarkerPos = pos;

            emit endMarkerPosChanged();
        }
    }
}


void GuiModel::setStartMarkerState(bool bState)
{
    if (_bStartMarkerState != bState)
    {
        _bStartMarkerState = bState;

        if (_bStartMarkerState && _bEndMarkerState)
        {
            setMarkerState(true);
        }

        if (!_bStartMarkerState && !_bEndMarkerState)
        {
             setMarkerState(false);
        }
    }
}

void GuiModel::setEndMarkerState(bool bState)
{
    if (_bEndMarkerState != bState)
    {
        _bEndMarkerState = bState;

        if (_bStartMarkerState && _bEndMarkerState)
        {
            setMarkerState(true);
        }

        if (!_bStartMarkerState && !_bEndMarkerState)
        {
            setMarkerState(false);
        }
    }
}

void GuiModel::setMarkerState(bool bState)
{
    // Always send signal, because we also need to clear markers when only one is visible */
    _bMarkerState = bState;

    emit markerStateChanged();
}

double GuiModel::yAxisMin()
{
    return _guiSettings.yMin;
}

double GuiModel::y2AxisMin()
{
    return _guiSettings.y2Min;
}

double GuiModel::yAxisMax()
{
    return _guiSettings.yMax;
}

double GuiModel::y2AxisMax()
{
    return _guiSettings.y2Max;
}
