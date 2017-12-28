#ifndef BASICGRAPHVIEW_H
#define BASICGRAPHVIEW_H

#include <QObject>
#include "myqcustomplot.h"


/* forward declaration */
class GuiModel;
class GraphDataModel;
class NoteModel;

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

    explicit BasicGraphView(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel, NoteModel * pNoteModel, MyQCustomPlot *pPlot, QObject *parent = 0);
    virtual ~BasicGraphView();

    qint32 graphDataSize();
    bool valuesUnderCursor(QList<double> &valueList);

    double pixelToKey(double pixel);
    double pixelToValue(double pixel);

public slots:

    virtual void manualScaleXAxis(qint64 min, qint64 max);
    virtual void manualScaleYAxis(qint64 min, qint64 max);

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
    virtual void setOpenGl(bool bState);
    virtual bool openGl(void);

    virtual void handleNoteValueDataChanged(const quint32 idx);
    virtual void handleNoteKeyDataChanged(const quint32 idx);
    virtual void handleNoteTextChanged(const quint32 idx);
    virtual void handleNoteAdded(const quint32 idx);
    virtual void handleNoteRemoved(const quint32 idx);

signals:
    void cursorValueUpdate();

private slots:
    void selectionChanged();

    void mousePress(QMouseEvent *event);
    void mouseRelease();
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
    QCPGraphDataContainer::const_iterator getClosestPoint(double xPos);

    QVector<QString> _tickLabels;
    QList<QCPItemText *> _notesItems;

    QCPItemStraightLine * _pStartMarker;
    QCPItemStraightLine * _pEndMarker;

    static const qint32 _cPixelPerPointThreshold = 5; /* in pixels */

};

#endif // BASICGRAPHVIEW_H
