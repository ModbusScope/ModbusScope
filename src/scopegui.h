#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>
#include <QVector>

#include "scopedata.h"
#include "mainwindow.h"
#include "datafileparser.h"
#include "qcustomplot.h"

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(MainWindow * window, ScopeData * scopedata, QCustomPlot * pPlot, QObject *parent);

    typedef enum
    {
        SCALE_AUTO = 0,
        SCALE_SLIDING,
        SCALE_MINMAX,
        SCALE_MANUAL
    } AxisScaleOptions;

    void resetGraph(void);
    void setupGraph(QList<RegisterData> registerList);
    void loadFileData(DataFileParser::FileData * pData);
    void setxAxisScale(AxisScaleOptions scaleMode);
    void setxAxisScale(AxisScaleOptions scaleMode, quint32 interval);

    void setyAxisScale(AxisScaleOptions scaleMode);
    void setyAxisScale(AxisScaleOptions scaleMode, qint32 min, qint32 max);

    void setyAxisMinMax(quint32 min, quint32 max);

    qint32 getyAxisMin();
    qint32 getyAxisMax();

    void bringToFront(quint32 index, bool bFront);
    void showGraph(quint32 index, bool bShow);
    QColor getGraphColor(quint32 index);
    void resetResults();

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

private:

    void writeToFile(QString filePath, QString logData);
    void scalePlot();
    QString createTickLabelString(qint32 tickKey);
    void highlightSamples(bool bState);

    typedef struct _GuiSettings
    {
        _GuiSettings() : scaleXSetting(SCALE_AUTO), scaleYSetting(SCALE_AUTO) {}

        AxisScaleOptions scaleXSetting;
        quint32 xslidingInterval;

        AxisScaleOptions scaleYSetting;
        qint32 yMin;
        qint32 yMax;

    } GuiSettings;

    QCustomPlot * _pPlot;

    static const QList<QColor> _colorlist;

    MainWindow * _window;
    ScopeData * _scopedata;

    QVector<QString> _tickLabels;
    QList<QString> _graphNames;

    GuiSettings _settings;
    bool _bEnableTooltip;
    bool _bEnableSampleHighlight;

    static const qint32 _cPixelNearThreshold = 20; /* in pixels */
    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */

};

#endif // SCOPEGUI_H
