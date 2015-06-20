#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>
#include <QVector>

#include "guimodel.h"

#include "scopedata.h"
#include "mainwindow.h"
#include "datafileparser.h"
#include "qcustomplot.h"

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(MainWindow * window, ScopeData * scopedata, QCustomPlot * pPlot, QObject *parent);

    void resetGraph(void);
    void setupGraph(QList<RegisterData> registerList);
    void loadFileData(DataFileParser::FileData * pData);


    qint32 getyAxisMin();
    qint32 getyAxisMax();

    void resetResults();
    void setLegendPosition(LegendsPositionOptions pos);

signals:
    void updateXScalingUi(int);
    void updateYScalingUi(int);

public slots:
    void plotResults(QList<bool> successList, QList<double> valueList);
    void setxAxisSlidingInterval(int interval);
    void exportDataCsv(QString dataFile);
    void exportGraphImage(QString imageFile);
    void enableValueTooltip(bool bState);
    void enableSamplePoints(bool bState);

private slots:
    void generateTickLabels();
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void mouseMove(QMouseEvent *event);
    void axisDoubleClicked(QCPAxis * axis);
    void handleSamplePoints();
    void xAxisRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);

private:

    void writeToFile(QString filePath, QString logData);
    void scalePlot();





};

#endif // SCOPEGUI_H
