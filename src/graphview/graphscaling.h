#ifndef GRAPHSCALING_H
#define GRAPHSCALING_H

#include <QObject>
#include "scopeplot.h" /* for QCPAxis definition  */

// Forward declaration
class ScopePlot;
class GuiModel;
class GraphView;

class GraphScale : public QObject
{
    Q_OBJECT
public:
    explicit GraphScale(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphScale();

    void rescale();

    void disableRangeZoom();
    void enableRangeZoom();
    void configureDragDirection();
    void zoomGraph();
    void handleDrag();

private slots:
    void selectionChanged();
    void axisDoubleClicked(QCPAxis* axis);

private:

    GuiModel* _pGuiModel;
    ScopePlot* _pPlot;
    GraphView* _pGraphview;

};

#endif // GRAPHSCALING_H
