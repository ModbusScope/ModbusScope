#ifndef EXTENDEDGRAPHVIEW_H
#define EXTENDEDGRAPHVIEW_H

#include <QObject>
#include "basicgraphview.h"

/* Forward declaration */
class SettingsModel;

class ExtendedGraphView : public BasicGraphView
{
    Q_OBJECT

public:
    ExtendedGraphView(GuiModel *pGuiModel, SettingsModel * pSettingsModel, GraphDataModel * pRegisterDataModel, NoteModel * pNoteModel, MyQCustomPlot *pPlot, QObject *parent);
    virtual ~ExtendedGraphView();

public slots:
    void addData(QList<double> timeData, QList<QList<double> > data);
    void showGraph(quint32 graphIdx);
    void rescalePlot();
    void plotResults(QList<bool> successList, QList<double> valueList);
    void clearResults();

signals:
    void dataAddedToPlot(double timeData, QList<double> dataList);

private slots:
    void updateData(QList<double> *pTimeData, QList<QList<double> > * pDataLists);
    void xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);

private:
    static const quint64 _cOptimizeThreshold = 1000000uL;

    SettingsModel * _pSettingsModel;

};

#endif // EXTENDEDGRAPHVIEW_H
