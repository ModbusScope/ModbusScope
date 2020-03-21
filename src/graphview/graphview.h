#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QObject>

#include "myqcustomplot.h"

/* forward declaration */
class GuiModel;
class GraphDataModel;
class NoteModel;
class SettingsModel;

class GraphViewZoom;
class NoteHandling;

class GraphView : public QObject
{
    Q_OBJECT
public:

    explicit GraphView(GuiModel *pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel * pNoteModel, MyQCustomPlot *pPlot, QObject *parent = nullptr);
    virtual ~GraphView();

    qint32 graphDataSize();
    bool valuesUnderCursor(QList<double> &valueList);

    QPointF pixelToPointF(const QPoint &pixel) const;
    double pixelToClosestKey(double pixel);
    double pixelToClosestValue(double pixel);

    void showMarkers();

public slots:

    void updateTooltip();
    void enableSamplePoints();
    void clearGraph(const quint32 graphIdx);
    void updateGraphs();
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphLabel(const quint32 graphIdx);
    void bringToFront();
    void updateMarkersVisibility();
    void setStartMarker();
    void setEndMarker();

    void addData(QList<double> timeData, QList<QList<double> > data);
    void showGraph(quint32 graphIdx);
    void rescalePlot();
    void plotResults(QList<bool> successList, QList<double> valueList);
    void clearResults();

signals:
    void cursorValueUpdate();
    void dataAddedToPlot(double timeData, QList<double> dataList);

private slots:
    void selectionChanged();

    void mousePress(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void mouseWheel();
    void mouseMove(QMouseEvent *event);

    void handleSamplePoints();
    void axisDoubleClicked(QCPAxis * axis);

    void updateData(QList<double> *pTimeData, QList<QList<double> > * pDataLists);
    void xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);

private:
    void paintTimeStampToolTip(QPoint pos);
    void highlightSamples(bool bState);
    qint32 graphIndex(QCPGraph * pGraph);
    double getClosestPoint(double coordinate);

    QVector<QString> _tickLabels;

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    MyQCustomPlot * _pPlot;
    bool _bEnableSampleHighlight;

    GraphViewZoom* _pGraphViewZoom;
    NoteHandling* _pNoteHandling;

    QCPItemStraightLine * _pStartMarker;
    QCPItemStraightLine * _pEndMarker;

    QPoint _tooltipLocation;

    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */
    static const quint64 _cOptimizeThreshold = 1000000uL;

};

#endif // GRAPHVIEW_H
