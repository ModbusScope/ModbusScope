#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>
#include <QVector>

#include "scopedata.h"
#include "mainwindow.h"

// Foward declaration
class QCustomPlot;

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
    void setupGraph(QList<QString> registerTextList);
    void setxAxisScale(AxisScaleOptions scaleMode);
    void setxAxisScale(AxisScaleOptions scaleMode, quint32 interval);

    void setyAxisScale(AxisScaleOptions scaleMode);
    void setyAxisScale(AxisScaleOptions scaleMode, qint32 min, qint32 max);

    void setyAxisMinMax(quint32 min, quint32 max);

    qint32 getyAxisMin();
    qint32 getyAxisMax();

signals:
    void updateXScalingUi(int);
    void updateYScalingUi(int);

public slots:
    void plotResults(QList<bool> successList, QList<quint16> valueList);
    void setxAxisSlidingInterval(int interval);
    void exportDataCsv(QString dataFile);
    void exportGraphImage(QString imageFile);

private slots:
    void generateTickLabels();
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void mouseMove(QMouseEvent *event);

private:

    void writeToFile(QString filePath, QString logData);
    void scalePlot();

    typedef struct
    {
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

    QVector<QString> tickLabels;
    QList<QString> graphNames;

    GuiSettings _settings;


    static const quint32 _cMinuteTripPoint = 5*60*1000; /* in ms */
    static const quint32 _cHourTripPoint = 10*60*60*1000; /* in ms */

};

#endif // SCOPEGUI_H
