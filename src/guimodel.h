#ifndef GUIMODEL_H
#define GUIMODEL_H

#include <QObject>
#include "graphdata.h"
#include "basicgraphview.h"
#include "registerdata.h"

class GuiModel : public QObject
{
    Q_OBJECT
public:

    explicit GuiModel(QObject *parent = 0);
    ~GuiModel();

    void triggerUpdate(void);

    bool graphVisibility(quint32 index) const;
    QColor graphColor(quint32 index) const;
    QString graphLabel(quint32 index) const;

    qint32 frontGraph() const;

    QString loadedFile() const; /* TODO */
    void setLoadedFile(const QString &loadedFile); /* TODO */

    bool highlightSamples() const;
    bool valueTooltip() const;
    void addGraphs(QList<RegisterData> registerList);
    void addGraphs(QList<RegisterData> registerList, QList<QList<double> > data);
    void clearGraph();
    quint32 graphCount();
    QString windowTitle();
    bool legendVisibility();
    QString projectFilePath();
    QString lastDataFilePath();
    BasicGraphView::AxisScaleOptions xAxisScalingMode();
    quint32 xAxisSlidingSec();
    BasicGraphView::AxisScaleOptions  yAxisScalingMode();
    qint32 yAxisMin();
    qint32 yAxisMax();

    void setProjectFilePath(QString path);
    void setLastDataFilePath(QString path);

public slots:
    void setValueTooltip(bool bValueTooltip);
    void setHighlightSamples(bool bHighlightSamples);
    void setFrontGraph(const qint32 &frontGraph);
    void setGraphVisibility(quint32 index, const bool &value);
    void setWindowTitleDetail(QString detail);
    void setLegendVisibility(bool bLegendVisibility);

    void setxAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setxAxisSlidingInterval(qint32 slidingSec);

    void setyAxisScale(BasicGraphView::AxisScaleOptions scaleMode);
    void setyAxisMin(qint32 newMin);
    void setyAxisMax(qint32 newMax);

signals:

    void graphVisibilityChanged(const quint32 index);
    void graphCleared();
    void graphsAdded();
    void graphsAddedWithData(QList<QList<double> > data);
    void frontGraphChanged();
    void loadedFileChanged();
    void highlightSamplesChanged();
    void valueTooltipChanged();
    void windowTitleChanged();
    void legendVisibilityChanged();
    void xAxisScalingChanged();
    void xAxisSlidingIntervalChanged();
    void yAxisScalingChanged();
    void yAxisMinMaxchanged();

private slots:

private:

    typedef struct
    {
        BasicGraphView::AxisScaleOptions xScaleMode;
        quint32 xslidingInterval;

        BasicGraphView::AxisScaleOptions yScaleMode;
        qint32 yMin;
        qint32 yMax;

    } GuiSettings;


    QColor getColor(const quint32 index);

    QList<GraphData * > _graphData;

    qint32 _frontGraph;
    GuiSettings _guiSettings;
    QString _loadedFile;
    QString _windowTitle;

    QString _projectFilePath;
    QString _lastDataFilePath;

    bool _bHighlightSamples;
    bool _bValueTooltip;
    bool _bLegendVisibility;

    static const QString _cWindowTitle;
    static const QList<QColor> _colorlist;

};

#endif // GUIMODEL_H
