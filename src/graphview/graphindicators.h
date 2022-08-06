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
    void add(QCPGraph* pGraph);

private slots:
    void axisRangeChanged(const QCPRange &newRange);
    void setTracerPosition(double key);

private:

    GraphDataModel* _pGraphDataModel;
    ScopePlot* _pPlot;
    QCPGraph* _pGraph;

    QList<QCPItemTracer *> _axisTracerList;

};

#endif // GRAPHINDICATOR_H
