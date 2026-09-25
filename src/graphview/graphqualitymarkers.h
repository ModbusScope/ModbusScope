#ifndef GRAPHQUALITYMARKERS_H
#define GRAPHQUALITYMARKERS_H

#include "graphview/scopeplot.h"
#include "models/graphdata.h"
#include "util/graphindex.h"
#include "util/result.h"

#include <QList>
#include <QMap>
#include <QObject>

// Forward declaration
class GraphDataModel;

class GraphQualityMarkers : public QObject
{
    Q_OBJECT
public:
    explicit GraphQualityMarkers(GraphDataModel* pGraphDataModel, ScopePlot* pPlot, QObject* parent = nullptr);
    virtual ~GraphQualityMarkers();

    void rebuild();
    void appendSample(GraphIdx graphIdx, double timestamp, double value, const DataQuality::Quality& quality);
    void setAxis(GraphIdx graphIdx, GraphData::valueAxis_t axis);
    void setVisible(GraphIdx graphIdx, bool bVisible);

private:
    void removeOverlays();
    void addOverlays(GraphIdx graphIdx);
    void loadSeries(GraphIdx graphIdx);

    GraphDataModel* _pGraphDataModel;
    ScopePlot* _pPlot;

    /* Overlay curves per graph index, ordered like the marked states they represent */
    QMap<GraphIdx, QList<QCPCurve*> > _overlays;
};

#endif // GRAPHQUALITYMARKERS_H
