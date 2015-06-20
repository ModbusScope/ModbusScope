#ifndef BASICGRAPHVIEW_H
#define BASICGRAPHVIEW_H

#include <QObject>
#include "qcustomplot.h"


/* forward declaration */
class GuiModel;

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

    typedef enum
    {
        LEGEND_LEFT = 0,
        LEGEND_MIDDLE,
        LEGEND_RIGHT,
    } LegendsPositionOptions;

    explicit BasicGraphView(GuiModel *pGuiModel, QCustomPlot *pPlot, QObject *parent = 0);
    virtual ~BasicGraphView();

    void exportGraphImage(QString imageFile);
    virtual void manualScaleXAxis(qint64 min, qint64 max);
    virtual void manualScaleYAxis(qint64 min, qint64 max);

public slots:
    virtual void autoScaleXAxis();
    virtual void autoScaleYAxis();

    virtual void enableValueTooltip();
    virtual void enableSamplePoints();
    virtual void clearGraphs();
    virtual void addGraphs();
    virtual void showHideLegend();
    virtual void showGraph(quint32 index);
    virtual void bringToFront();
    virtual void setLegendPosition(LegendsPositionOptions pos);

signals:

private slots:
    void generateTickLabels();
    void selectionChanged();

    void mousePress();
    void mouseWheel();
    void legendClick(QCPLegend * legend, QCPAbstractLegendItem * abstractLegendItem, QMouseEvent * event);
    void legendDoubleClick(QCPLegend * legend,QCPAbstractLegendItem * abstractLegendItem, QMouseEvent * event);

protected:
    GuiModel * _pGuiModel;
    QCustomPlot * _pPlot;
    bool _bEnableTooltip;
    bool _bEnableSampleHighlight;

    virtual void paintValueToolTip(QMouseEvent *event);
    virtual void handleSamplePoints();
    virtual void axisDoubleClicked(QCPAxis * axis);

private:
    QString createTickLabelString(qint32 tickKey);
    void highlightSamples(bool bState);
    qint32 getGraphIndex(QCPGraph * pGraph);

    QVector<QString> tickLabels;

    static const qint32 _cPixelNearThreshold = 20; /* in pixels */
    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */


};

#endif // BASICGRAPHVIEW_H
