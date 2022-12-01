#ifndef GUIMODEL_H
#define GUIMODEL_H

#include <QObject>

namespace AxisMode
{
    typedef enum
    {
        SCALE_AUTO = 0,
        SCALE_WINDOW_AUTO,
        SCALE_SLIDING,
        SCALE_MINMAX,
        SCALE_MANUAL
    } AxisScaleOptions;
}

class GuiModel : public QObject
{
    Q_OBJECT
public:

    explicit GuiModel(QObject *parent = nullptr);
    ~GuiModel();

    static const quint32 cDifferenceMask;
    static const quint32 cSlopeMask;
    static const quint32 cAverageMask;
    static const quint32 cMinimumMask;
    static const quint32 cMaximumMask;

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

    typedef enum
    {
        ZOOM_IDLE = 0,
        ZOOM_TRIGGERED, /* Zoom has been enabled */
        ZOOM_SELECTING, /* Zomm rectangle is selecting */
    } ZoomState;

    void triggerUpdate(void);

    qint32 frontGraph() const;
    bool highlightSamples() const;
    bool cursorValues() const;
    QString windowTitle();
    QString projectFilePath();
    QString lastDir();
    QString lastMbcImportedFile();
    AxisMode::AxisScaleOptions xAxisScalingMode();
    quint32 xAxisSlidingSec();
    AxisMode::AxisScaleOptions yAxisScalingMode();
    AxisMode::AxisScaleOptions y2AxisScalingMode();
    double yAxisMin();
    double y2AxisMin();
    double yAxisMax();
    double y2AxisMax();
    quint32 guiState();
    qint64 communicationStartTime();
    qint64 communicationEndTime();
    quint32 communicationErrorCount();
    quint32 communicationSuccessCount();
    double startMarkerPos();
    double endMarkerPos();
    bool markerState();
    quint32 markerExpressionMask();
    ZoomState zoomState();

    void setProjectFilePath(QString path);
    void setLastDir(QString dir);
    void setLastMbcImportedFile(QString file);
    void setMarkerExpressionMask(quint32 mask);

    void incrementCommunicationStats(quint32 successCount, quint32 errors);

public slots:
    void setCursorValues(bool bCursorValues);
    void setHighlightSamples(bool bHighlightSamples);
    void setFrontGraph(const qint32 &frontGraph);

    void setWindowTitleDetail(QString detail);
    void setxAxisScale(AxisMode::AxisScaleOptions scaleMode);
    void setxAxisSlidingInterval(int slidingSec);
    void setyAxisScale(AxisMode::AxisScaleOptions scaleMode);
    void sety2AxisScale(AxisMode::AxisScaleOptions scaleMode);
    void setyAxisMin(double newMin);
    void sety2AxisMin(double newMin);
    void setyAxisMax(double newMax);
    void sety2AxisMax(double newMax);
    void setGuiState(quint32 state);
    void setCommunicationStartTime(qint64 startTime);
    void setCommunicationEndTime(qint64 endTime);
    void setCommunicationStats(quint32 successCount, quint32 errorCount);
    void clearMarkersState(void);
    void setStartMarkerPos(double pos);
    void setEndMarkerPos(double pos);
    void setZoomState(GuiModel::ZoomState zoomState);

signals:

    void frontGraphChanged();
    void highlightSamplesChanged();
    void cursorValuesChanged();
    void windowTitleChanged();
    void xAxisScalingChanged();
    void xAxisSlidingIntervalChanged();
    void yAxisScalingChanged();
    void y2AxisScalingChanged();
    void yAxisMinMaxchanged();
    void y2AxisMinMaxchanged();
    void guiStateChanged();
    void projectFilePathChanged();
    void communicationStatsChanged();
    void markerStateChanged();
    void startMarkerPosChanged();
    void endMarkerPosChanged();
    void markerExpressionMaskChanged();
    void zoomStateChanged();

    void yAxisSelected();
    void y2AxisSelected();

private slots:

private:

    void setStartMarkerState(bool bState);
    void setEndMarkerState(bool bState);
    void setMarkerState(bool bState);

    typedef struct
    {
        AxisMode::AxisScaleOptions xScaleMode;
        quint32 xslidingInterval;

        AxisMode::AxisScaleOptions yScaleMode;
        double yMin;
        double yMax;

        AxisMode::AxisScaleOptions y2ScaleMode;
        qint32 y2Min;
        qint32 y2Max;

    } GuiSettings;

    qint32 _frontGraph; /* active graph index */
    GuiSettings _guiSettings;
    QString _windowTitle;

    qint64 _startTime;
    qint64 _endTime;
    quint32 _successCount;
    quint32 _errorCount;

    QString _projectFilePath;
    QString _lastDir; // Last directory opened for import/export/load project
    QString _lastMbcImportedFile;

    bool _bHighlightSamples;
    bool _bCursorValues;
    quint32 _guiState;

    bool _bMarkerState;
    bool _bStartMarkerState;
    double _startMarkerPos;

    bool _bEndMarkerState;
    double _endMarkerPos;

    ZoomState _zoomState;

    quint32 _markerExpressionMask;

    static const QString _cWindowTitle;

};

#endif // GUIMODEL_H
