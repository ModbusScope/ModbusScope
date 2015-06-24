#ifndef EXTENDEDGRAPHVIEW_H
#define EXTENDEDGRAPHVIEW_H

#include <QObject>
#include "basicgraphview.h"

/* Forward declaration */
class ScopeData;

class ExtendedGraphView : public BasicGraphView
{
    Q_OBJECT

public:
    ExtendedGraphView(ScopeData * pScope, GuiModel *pGuiModel, QCustomPlot *pPlot, QObject *parent);
    virtual ~ExtendedGraphView();

    QList<double> graphTimeData();
    QList<QCPData> graphData(qint32 index);

public slots:
    void addGraphs(QList<QList<double> > data);
    void plotResults(QList<bool> successList, QList<double> valueList);
    void clearResults();
    void rescalePlot();

private slots:
    void mouseMove(QMouseEvent *event);
    void updateData(QList<QList<double> > * pDataLists);
    void xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);

private:

    ScopeData * _pScope;

};

#endif // EXTENDEDGRAPHVIEW_H
