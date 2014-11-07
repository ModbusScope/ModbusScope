#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>
#include <QVector>

#include "mainwindow.h"

// Foward declaration
class QCustomPlot;

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(MainWindow * window, QCustomPlot * pPlot, QObject *parent);

    void resetGraph(void);
    void addGraph(quint16 registerAddress);
    void setxAxisAutoScale();
    void setxAxisSlidingScale(quint32 interval);
    void setxAxisManualScale();

signals:
    void updateXScalingUi(int);

public slots:
    void plotResults(QList<bool> successList, QList<quint16> valueList);
    void setYAxisAutoScale(int state);
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

    typedef enum
    {
        SCALE_AUTO = 0,
        SCALE_SLIDING,
        SCALE_MANUAL
    } XAxisScaleOptions;

    typedef struct
    {
        XAxisScaleOptions scaleXSetting;
        quint32 xslidingInterval;
        bool bYAxisAutoScale;
    } GuiSettings;

    QCustomPlot * _pPlot;
    qint64 _startTime;

    static const QList<QColor> _colorlist;

    MainWindow * _window;

    QVector<QString> tickLabels;
    QList<QString> graphNames;

    GuiSettings _settings;

    static const quint32 _cMinuteTripPoint = 5*60*1000; /* in ms */
    static const quint32 _cHourTripPoint = 10*60*60*1000; /* in ms */

};

#endif // SCOPEGUI_H
