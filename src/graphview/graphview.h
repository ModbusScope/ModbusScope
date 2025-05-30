#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include "models/graphdata.h"
#include "util/result.h"

#include <QObject>

/* forward declaration */
class GuiModel;
class GraphDataModel;
class NoteModel;
class SettingsModel;
class GraphScale;
class GraphViewZoom;
class GraphMarkers;
class GraphIndicators;
class NoteHandling;
class ScopePlot;

class GraphView : public QObject
{
    Q_OBJECT
public:

    explicit GraphView(GuiModel *pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel * pNoteModel, ScopePlot *pPlot, QObject *parent = nullptr);
    virtual ~GraphView();

    qint32 graphDataSize();
    bool valuesUnderCursor(QList<double> &valueList);

    QPointF pixelToPointF(const QPoint &pixel) const;
    double pixelToClosestKey(double pixel);

    void showMarkers();

public slots:

    void updateTooltip();
    void enableSamplePoints();
    void clearGraph(const quint32 graphIdx);
    void updateGraphs();
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphAxis(const quint32 graphIdx);
    void changeSelectedGraph(const qint32 activeGraphIdx);
    void bringToFront(const qint32 activeGraphIdx);

    void addData(QList<double> timeData, QList<QList<double> > data);
    void handleGraphVisibilityChange(quint32 graphIdx);
    void rescalePlot();
    void plotResults(ResultDoubleList resultList);
    void clearResults();

signals:
    void cursorValueUpdate();
    void dataAddedToPlot(double timeData, QList<double> dataList);
    void afterGraphUpdate();

private slots:
    void mousePress(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void mouseWheel();
    void mouseMove(QMouseEvent *event);
    void handleSelectionChanged(bool selected);

    void handleSamplePoints();

private:
    void paintTimeStampToolTip(QPoint pos);
    void highlightSamples(bool bState);
    void setGraphColor(QCPGraph* _pGraph, const QColor &color);
    void setGraphAxis(QCPGraph* _pGraph, const GraphData::valueAxis_t &axis);
    double getClosestPoint(double coordinate);
    void updateSecondaryAxisVisibility();
    qint32 getActiveGraphIndex(const QCPGraph * const pGraph);

    QVector<QString> _tickLabels;

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    ScopePlot * _pPlot;
    bool _bEnableSampleHighlight;

    GraphScale* _pGraphScale;
    GraphViewZoom* _pGraphViewZoom;
    GraphMarkers* _pGraphMarkers;
    GraphIndicators* _pGraphIndicators;
    NoteHandling* _pNoteHandling;

    QPoint _tooltipLocation;

    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */
    static const quint64 _cOptimizeThreshold = 1000000uL;

};

#endif // GRAPHVIEW_H
