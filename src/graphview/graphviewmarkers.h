#ifndef GRAPHVIEWMARKERS_H
#define GRAPHVIEWMARKERS_H

#include <QObject>
#include "myqcustomplot.h"

// Forward declaration
class GuiModel;
class GraphView;

class GraphViewMarkers : public QObject
{
    Q_OBJECT
public:
    explicit GraphViewMarkers(GuiModel* pGuiModel, MyQCustomPlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphViewMarkers();

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
    MyQCustomPlot* _pPlot;
    GraphView* _pGraphview;

    QList<QCPItemTracer *> _startTracerList;
    QList<QCPItemTracer *> _endTracerList;

    QCPItemStraightLine* _pStartMarker;
    QCPItemStraightLine* _pEndMarker;

};

#endif // GRAPHVIEWMARKERS_H
