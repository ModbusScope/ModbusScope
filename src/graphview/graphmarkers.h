#ifndef GRAPHMARKERS_H
#define GRAPHMARKERS_H

#include <QObject>
#include "scopeplot.h"

// Forward declaration
class GuiModel;
class GraphView;

class GraphMarkers : public QObject
{
    Q_OBJECT
public:
    explicit GraphMarkers(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphMarkers();

    void clearTracers();
    void addTracer(QCPGraph* pGraph);
    void updateTracersVisibility();

private slots:
    void updateMarkersVisibility();
    void setStartMarker();
    void setEndMarker();

private:

    void setTracerVisibility(QList<QCPItemTracer *> &tracerList, bool bMarkerVisibility);
    void setTracerPosition(QList<QCPItemTracer *> &tracerList, double pos);
    QCPItemTracer* createTracer(QCPGraph* pGraph);

    GuiModel* _pGuiModel;
    ScopePlot* _pPlot;
    GraphView* _pGraphview;

    QList<QCPItemTracer *> _startTracerList;
    QList<QCPItemTracer *> _endTracerList;

    QCPItemStraightLine* _pStartMarker;
    QCPItemStraightLine* _pEndMarker;

};

#endif // GRAPHMARKERS_H
