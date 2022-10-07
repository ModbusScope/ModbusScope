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
    void updateVisibility();

private slots:
    void setTracerPosition(const QCPRange &newRange);
    void updateColor(quint32 graphIdx);
    void updateValueAxis(quint32 graphIdx);

private:
    void configureValueAxis(quint32 graphIdx);

    GraphDataModel* _pGraphDataModel;
    ScopePlot* _pPlot;

    QList<QCPItemTracer *> _valueTracers;
    QList<QCPItemTracer *> _axisValueTracers;

};

#endif // GRAPHINDICATOR_H
