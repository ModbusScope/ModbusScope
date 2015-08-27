#ifndef GUIMODEL_H
#define GUIMODEL_H

#include <QObject>
#include "graphdata.h"
#include "basicgraphview.h"
#include "registerdata.h"

class GuiModel : public QObject
{
    Q_OBJECT
public:

    explicit GuiModel(QObject *parent = 0);
    ~GuiModel();

    enum
    {
        INIT,
        STARTED,
        STOPPED,
    };

    void triggerUpdate(void);

    bool graphVisibility(quint32 index) const;
    QColor graphColor(quint32 index) const;
    QString graphLabel(quint32 index) const;
    qint32 frontGraph() const;
    bool highlightSamples() const;
    bool valueTooltip() const;
    void addGraphs(QList<RegisterData> registerList);
    void addGraphs(QList<RegisterData> registerList, QList<double> timeData, QList<QList<double> > data);
    void clearGraph();
    quint32 graphCount();
    QString windowTitle();
    bool legendVisibility();
    BasicGraphView::LegendsPositionOptions legendPosition();
    QString projectFilePath();
    QString dataFilePath();
    QString lastDir();
    BasicGraphView::AxisScaleOptions xAxisScalingMode();
    quint32 xAxisSlidingSec();
    BasicGraphView::AxisScaleOptions  yAxisScalingMode();
    qint32 yAxisMin();
    qint32 yAxisMax();
    quint32 communicationState();
    qint64 communicationStartTime();
    qint64 communicationEndTime();
    quint32 communicationErrorCount();
    quint32 communicationSuccessCount();

    void setProjectFilePath(QString path);
    void setDataFilePath(QString path);
    void setLastDir(QString dir);
    void setGraphReset(bool bGraphReset);
    bool graphReset();

public slots:
    void setValueTooltip(bool bValueTooltip);
    void setHighlightSamples(bool bHighlightSamples);
    void setFrontGraph(const qint32 &frontGraph);
    void setGraphVisibility(quint32 index, const bool &value);
    void setWindowTitleDetail(QString detail);
    void setLegendVisibility(bool bLegendVisibility);
    void setLegendPosition(BasicGraphView::LegendsPositionOptions pos);
    void setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setxAxisSlidingInterval(qint32 slidingSec);
    void setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setyAxisMin(qint32 newMin);
    void setyAxisMax(qint32 newMax);
    void setCommunicationState(quint32 state);
    void setCommunicationStartTime(qint64 startTime);
    void setCommunicationEndTime(qint64 endTime);
    void setCommunicationStats(quint32 successCount, quint32 errorCount);

signals:

    void graphVisibilityChanged(const quint32 index);
    void graphCleared();
    void graphsAdded();
    void graphsAddData(QList<double>, QList<QList<double> > data);
    void frontGraphChanged();
    void highlightSamplesChanged();
    void valueTooltipChanged();
    void windowTitleChanged();
    void legendVisibilityChanged();
    void legendPositionChanged();
    void xAxisScalingChanged();
    void xAxisSlidingIntervalChanged();
    void yAxisScalingChanged();
    void yAxisMinMaxchanged();
    void communicationStateChanged();
    void projectFilePathChanged();
    void dataFilePathChanged();
    void communicationStatsChanged();

private slots:

private:

    typedef struct
    {
        BasicGraphView::AxisScaleOptions xScaleMode;
        quint32 xslidingInterval;

        BasicGraphView::AxisScaleOptions yScaleMode;
        qint32 yMin;
        qint32 yMax;

    } GuiSettings;

    QList<GraphData * > _graphData;

    qint32 _frontGraph;
    GuiSettings _guiSettings;
    QString _windowTitle;

    qint64 _startTime;
    qint64 _endTime;
    quint32 _successCount;
    quint32 _errorCount;
    bool _bGraphReset;

    QString _projectFilePath;
    QString _dataFilePath;
    QString _lastDir; // Last directory opened for import/export/load project

    bool _bHighlightSamples;
    bool _bValueTooltip;
    bool _bLegendVisibility;
    BasicGraphView::LegendsPositionOptions _legendPosition;
    quint32 _communicationState;

    static const QString _cWindowTitle;

};

#endif // GUIMODEL_H
