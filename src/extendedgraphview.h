#ifndef EXTENDEDGRAPHVIEW_H
#define EXTENDEDGRAPHVIEW_H

#include <QObject>
#include "basicgraphview.h"

class ExtendedGraphView : public BasicGraphView
{
    Q_OBJECT

public:
    ExtendedGraphView(GuiModel *pGuiModel, QCustomPlot *pPlot, QObject *parent);
    virtual ~ExtendedGraphView();

public slots:
    void addGraphs(QList<QList<double> > data);
    void plotResults(QList<bool> successList, QList<double> valueList);
    void clearResults();
    void exportDataCsv(QString dataFile);
    void rescalePlot();

private slots:
    void mouseMove(QMouseEvent *event);
    void updateData(QList<QList<double> > * pDataLists);


private:
    void writeToFile(QString filePath, QString logData);

};

#endif // EXTENDEDGRAPHVIEW_H
