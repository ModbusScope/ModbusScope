#ifndef GRAPHINDICATOR_H
#define GRAPHINDICATOR_H

#include <QObject>
#include "scopeplot.h"

// Forward declaration
class GraphDataModel;

class GraphIndicators : public QObject
{
    Q_OBJECT
public:
    explicit GraphIndicators(GraphDataModel * pGraphDataModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphIndicators();

    void clear();
    void add(quint32 graphIdx, QCPGraph* pGraph);
    void setFrontGraph(quint32 graphIdx);
    void updateIndicatorVisibility();

private slots:
    void setTracerPosition(const QCPRange &newRange);
    void updateColor(quint32 graphIdx);
    void updateValueAxis(quint32 graphIdx);

private:
    void setTracerPosition();
    void configureValueAxis(quint32 graphIdx);
    void updateVisibility();
    bool determineVisibility(uint32_t activeIdx);

    GraphDataModel* _pGraphDataModel;
    ScopePlot* _pPlot;

    QList<QCPItemTracer *> _valueTracers;
    QList<QCPItemTracer *> _axisValueTracers;

};

#endif // GRAPHINDICATOR_H
