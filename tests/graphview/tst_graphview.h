#ifndef TST_GRAPHVIEW_H
#define TST_GRAPHVIEW_H

#include "qcustomplot/qcustomplot.h"

#include <QObject>

class GuiModel;
class SettingsModel;
class GraphDataModel;
class CommunicationStatsModel;
class NoteModel;
class ScopePlot;
class GraphView;
class QWidget;

class TestGraphView : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void clearGraphWithMultipleActiveGraphsResetsQualityToNoValue();

    void qualityMarkersBucketSamplesByState();
    void qualityMarkersShowPaddedSamplesAsNoValue();
    void qualityMarkersAppendLiveSample();
    void qualityMarkersFollowGraphVisibility();

private:
    QList<QCPCurve*> markerCurves() const;
    QCPCurve* markerCurve(QCPScatterStyle::ScatterShape shape) const;
    int markerCount(QCPScatterStyle::ScatterShape shape) const;

    QWidget* _pHost = nullptr;
    ScopePlot* _pPlot = nullptr;
    GuiModel* _pGuiModel = nullptr;
    SettingsModel* _pSettingsModel = nullptr;
    GraphDataModel* _pGraphDataModel = nullptr;
    CommunicationStatsModel* _pCommunicationStatsModel = nullptr;
    NoteModel* _pNoteModel = nullptr;
    GraphView* _pGraphView = nullptr;
};

#endif // TST_GRAPHVIEW_H
