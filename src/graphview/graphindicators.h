#ifndef GRAPHINDICATOR_H
#define GRAPHINDICATOR_H

#include <QObject>
#include "scopeplot.h"
#include "util/graphindex.h"

// Forward declaration
class GraphDataModel;

class GraphIndicators : public QObject
{
    Q_OBJECT
public:
    explicit GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphIndicators();

    void clear();
    void add(GraphIdx graphIdx, QCPGraph* pGraph);
    void setFrontGraph(GraphIdx graphIdx);
    void updateIndicatorVisibility();

private slots:
    void setTracerPosition(const QCPRange &newRange);
    void updateColor(GraphIdx graphIdx);
    void updateValueAxis(GraphIdx graphIdx);

private:
    void setTracerPosition();
    void configureValueAxis(GraphIdx graphIdx);
    void updateVisibility();
    bool determineVisibility(ActiveIdx activeIdx);

    GraphDataModel* _pGraphDataModel;
    ScopePlot* _pPlot;

    QList<QCPItemTracer *> _valueTracers;
    QList<QCPItemTracer *> _axisValueTracers;

};

#endif // GRAPHINDICATOR_H
