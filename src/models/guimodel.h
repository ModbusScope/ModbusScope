#ifndef GUIMODEL_H
#define GUIMODEL_H

#include <QObject>
#include "basicgraphview.h"

class GuiModel : public QObject
{
    Q_OBJECT
public:

    explicit GuiModel(QObject *parent = 0);
    ~GuiModel();

    static const quint32 cDifferenceMask;
    static const quint32 cSlopeMask;
    static const quint32 cAverageMask;
    static const quint32 cMinimumMask;
    static const quint32 cMaximumMask;
    static const quint32 cCustomMask;

    static const QStringList cMarkerExpressionStrings;
    static const QList<quint32> cMarkerExpressionBits;

    static const QString cMarkerExpressionStart;
    static const QString cMarkerExpressionEnd;

    enum
    {
        INIT,
        STARTED,
        STOPPED,
        DATA_LOADED,
    };

    void triggerUpdate(void);

    qint32 frontGraph() const;
    bool highlightSamples() const;
    bool cursorValues() const;
    QString windowTitle();
    QString projectFilePath();
    QString dataFilePath();
    QString lastDir();
    BasicGraphView::AxisScaleOptions xAxisScalingMode();
    quint32 xAxisSlidingSec();
    BasicGraphView::AxisScaleOptions  yAxisScalingMode();
    qint32 yAxisMin();
    qint32 yAxisMax();
    quint32 guiState();
    qint64 communicationStartTime();
    qint64 communicationEndTime();
    quint32 communicationErrorCount();
    quint32 communicationSuccessCount();
    double startMarkerPos();
    double endMarkerPos();
    bool markerState();
    quint32 markerExpressionMask();
    QString markerExpressionCustomScript();

    void setProjectFilePath(QString path);
    void setDataFilePath(QString path);
    void setLastDir(QString dir);
    void setMarkerExpressionMask(quint32 mask);
    void setMarkerExpressionCustomScript(QString path);

public slots:
    void setCursorValues(bool bCursorValues);
    void setHighlightSamples(bool bHighlightSamples);
    void setFrontGraph(const qint32 &frontGraph);

    void setWindowTitleDetail(QString detail);
    void setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setxAxisSlidingInterval(qint32 slidingSec);
    void setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setyAxisMin(qint32 newMin);
    void setyAxisMax(qint32 newMax);
    void setGuiState(quint32 state);
    void setCommunicationStartTime(qint64 startTime);
    void setCommunicationEndTime(qint64 endTime);
    void setCommunicationStats(quint32 successCount, quint32 errorCount);
    void clearMarkersState(void);
    void setStartMarkerPos(double pos);
    void setEndMarkerPos(double pos);

signals:

    void frontGraphChanged();
    void highlightSamplesChanged();
    void cursorValuesChanged();
    void windowTitleChanged();
    void xAxisScalingChanged();
    void xAxisSlidingIntervalChanged();
    void yAxisScalingChanged();
    void yAxisMinMaxchanged();
    void guiStateChanged();
    void projectFilePathChanged();
    void dataFilePathChanged();
    void communicationStatsChanged();
    void markerStateChanged();
    void startMarkerPosChanged();
    void endMarkerPosChanged();
    void markerExpressionMaskChanged();
    void markerExpressionCustomScriptChanged();

private slots:

private:

    void setStartMarkerState(bool bState);
    void setEndMarkerState(bool bState);
    void setMarkerState(bool bState);

    typedef struct
    {
        BasicGraphView::AxisScaleOptions xScaleMode;
        quint32 xslidingInterval;

        BasicGraphView::AxisScaleOptions yScaleMode;
        qint32 yMin;
        qint32 yMax;

    } GuiSettings;

    qint32 _frontGraph;
    GuiSettings _guiSettings;
    QString _windowTitle;

    qint64 _startTime;
    qint64 _endTime;
    quint32 _successCount;
    quint32 _errorCount;

    QString _projectFilePath;
    QString _dataFilePath;
    QString _lastDir; // Last directory opened for import/export/load project

    bool _bHighlightSamples;
    bool _bCursorValues;
    quint32 _guiState;

    bool _bMarkerState;
    bool _bStartMarkerState;
    double _startMarkerPos;

    bool _bEndMarkerState;
    double _endMarkerPos;

    quint32 _markerExpressionMask;
    QString _markerExpressionCustomScript;

    static const QString _cWindowTitle;

};

#endif // GUIMODEL_H
