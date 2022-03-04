#ifndef GRAPHVIEWZOOM_H
#define GRAPHVIEWZOOM_H

#include <QObject>
#include <QRubberBand>

// Forward declaration
class GraphView;
class GuiModel;
class ScopePlot;

class GraphViewZoom : public QObject
{
    Q_OBJECT
public:
    explicit GraphViewZoom(GuiModel* pGuiModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphViewZoom();

    bool handleMousePress(QMouseEvent *event);
    bool handleMouseRelease();
    bool handleMouseWheel();
    bool handleMouseMove(QMouseEvent *event);

private slots:
    void handleZoomStateChanged();

private:

    void performZoom(void);

    GuiModel* _pGuiModel;
    ScopePlot* _pPlot;
    GraphView* _pGraphview;

    QPoint _selectionOrigin;
    QRubberBand* _pRubberBand;
};

#endif // GRAPHVIEWZOOM_H
