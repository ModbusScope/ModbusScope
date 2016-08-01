#ifndef BASICGRAPHVIEW_H
#define BASICGRAPHVIEW_H

#include <QObject>
#include "myqcustomplot.h"


/* forward declaration */
class GuiModel;
class GraphDataModel;

class BasicGraphView : public QObject
{
    Q_OBJECT
public:

    typedef enum
    {
        SCALE_AUTO = 0,
        SCALE_SLIDING,
        SCALE_MINMAX,
        SCALE_MANUAL
    } AxisScaleOptions;

    explicit BasicGraphView(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel, MyQCustomPlot *pPlot, QObject *parent = 0);
    virtual ~BasicGraphView();

public slots:

    virtual void manualScaleXAxis(qint64 min, qint64 max);
    virtual void manualScaleYAxis(qint64 min, qint64 max);

    virtual void autoScaleXAxis();
    virtual void autoScaleYAxis();

    virtual void enableValueTooltip();
    virtual void enableSamplePoints();
    virtual void clearGraph(const quint32 graphIdx);
    virtual void updateGraphs();
    virtual void showGraph(quint32 graphIdx);
    virtual void changeGraphColor(const quint32 graphIdx);
    virtual void changeGraphLabel(const quint32 graphIdx);
    virtual void bringToFront();
    virtual void updateMarkersVisibility();
    virtual void setStartMarker();
    virtual void setEndMarker();

signals:

private slots:
    void generateTickLabels();
    void selectionChanged();

    void mousePress(QMouseEvent *event);
    void mouseRelease();
    void mouseWheel();
    void mouseMove(QMouseEvent *event);
    void paintValueToolTip(QMouseEvent *event);

protected slots:
    virtual void handleSamplePoints();
    virtual void axisDoubleClicked(QCPAxis * axis);

protected:
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    MyQCustomPlot * _pPlot;
    bool _bEnableTooltip;
    bool _bEnableSampleHighlight;

private:
    void highlightSamples(bool bState);
    qint32 graphIndex(QCPGraph * pGraph);
    bool smallScaleActive(QVector<double> tickList);

    QVector<QString> tickLabels;

    QCPItemStraightLine * _pStartMarker;
    QCPItemStraightLine * _pEndMarker;

    static const qint32 _cPixelNearThreshold = 20; /* in pixels */
    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */

    static const quint32 _cSmallScaleDiff = 2000;

};

#endif // BASICGRAPHVIEW_H
