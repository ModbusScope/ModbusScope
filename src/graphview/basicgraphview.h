#ifndef BASICGRAPHVIEW_H
#define BASICGRAPHVIEW_H

#include <QObject>

#include "myqcustomplot.h"
#include "noteitem.h"


/* forward declaration */
class GuiModel;
class GraphDataModel;
class NoteModel;
class GraphViewZoom;

class BasicGraphView : public QObject
{
    Q_OBJECT
public:

    typedef enum
    {
        SCALE_AUTO = 0,
        SCALE_WINDOW_AUTO,
        SCALE_SLIDING,
        SCALE_MINMAX,
        SCALE_MANUAL
    } AxisScaleOptions;

    explicit BasicGraphView(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel, NoteModel * pNoteModel, MyQCustomPlot *pPlot, QObject *parent = nullptr);
    virtual ~BasicGraphView();

    qint32 graphDataSize();
    bool valuesUnderCursor(QList<double> &valueList);

    QPointF pixelToPointF(const QPoint &pixel) const;
    double pixelToClosestKey(double pixel);
    double pixelToClosestValue(double pixel);

    void showMarkers();

public slots:

    virtual void autoScaleXAxis();
    virtual void autoScaleYAxis();

    virtual void updateTooltip();
    virtual void enableSamplePoints();
    virtual void clearGraph(const quint32 graphIdx);
    virtual void updateGraphs();
    virtual void changeGraphColor(const quint32 graphIdx);
    virtual void changeGraphLabel(const quint32 graphIdx);
    virtual void bringToFront();
    virtual void updateMarkersVisibility();
    virtual void setStartMarker();
    virtual void setEndMarker();

    virtual void handleNotePositionChanged(const quint32 idx);
    virtual void handleNoteTextChanged(const quint32 idx);
    virtual void handleNoteAdded(const quint32 idx);
    virtual void handleNoteRemoved(const quint32 idx);

signals:
    void cursorValueUpdate();

private slots:
    void selectionChanged();

    void mousePress(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void mouseWheel();
    void mouseMove(QMouseEvent *event);

protected slots:
    virtual void handleSamplePoints();
    virtual void axisDoubleClicked(QCPAxis * axis);

protected:
    void paintTimeStampToolTip(QPoint pos);

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    NoteModel * _pNoteModel;
    MyQCustomPlot * _pPlot;
    bool _bEnableSampleHighlight;

private:
    void highlightSamples(bool bState);
    qint32 graphIndex(QCPGraph * pGraph);
    double getClosestPoint(double coordinate);

    QVector<QString> _tickLabels;
    QList< QSharedPointer<NoteItem> > _notesItems;

    quint32 _pDraggedNoteIdx;
    QPoint _pixelOffset;

    GraphViewZoom* _pGraphViewZoom;

    QCPItemStraightLine * _pStartMarker;
    QCPItemStraightLine * _pEndMarker;

    QPoint _tooltipLocation;

    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */

};

#endif // BASICGRAPHVIEW_H
