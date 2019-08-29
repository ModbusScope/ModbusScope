#ifndef GRAPHVIEWZOOM_H
#define GRAPHVIEWZOOM_H

#include <QObject>
#include "myqcustomplot.h"
#include "guimodel.h"

class GraphViewZoom : public QObject
{
    Q_OBJECT

public:
    GraphViewZoom(GuiModel* pGuiModel, MyQCustomPlot* pPlot);
    ~GraphViewZoom();

private slots:
    void handleZoomStateChanged();

private:
    GuiModel* _pGuiModel;
    MyQCustomPlot* _pPlot;

};

#endif // GRAPHVIEWZOOM_H
