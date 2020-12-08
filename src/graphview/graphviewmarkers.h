#ifndef GRAPHVIEWMARKERS_H
#define GRAPHVIEWMARKERS_H

#include <QObject>
#include "myqcustomplot.h"
#include "guimodel.h"

// Forward declaration
class GraphView;

class GraphViewMarkers : public QObject
{
    Q_OBJECT
public:
    explicit GraphViewMarkers(GuiModel* pGuiModel, MyQCustomPlot* pPlot, QObject *parent = nullptr);
    virtual ~GraphViewMarkers();

private slots:
    void updateMarkersVisibility();
    void setStartMarker();
    void setEndMarker();

private:
    GuiModel* _pGuiModel;
    MyQCustomPlot* _pPlot;
    GraphView* _pGraphview;

    QCPItemStraightLine* _pStartMarker;
    QCPItemStraightLine* _pEndMarker;

};

#endif // GRAPHVIEWMARKERS_H
