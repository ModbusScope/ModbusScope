#ifndef GRAPHMENUCONTROLLER_H
#define GRAPHMENUCONTROLLER_H

#include "util/graphindex.h"

#include <QObject>

class QMenu;
class QAction;
class QColor;
class QIcon;
class GraphDataModel;
class OverlayLabel;

class GraphMenuController : public QObject
{
    Q_OBJECT

public:
    explicit GraphMenuController(GraphDataModel* pGraphDataModel,
                                 QMenu* pGraphMenu,
                                 OverlayLabel* pOverlayLabel,
                                 QObject* parent = nullptr);

public slots:
    void handleGraphsCountChanged();

signals:
    void activeGraphsChanged(bool bActiveGraphs);

private slots:
    void rebuildGraphMenu();
    void handleGraphVisibilityChange(GraphIdx graphIdx);
    void handleGraphColorChange(GraphIdx graphIdx);
    void handleGraphLabelChange(GraphIdx graphIdx);
    void menuShowHideGraphClicked(bool bState);

private:
    QAction* graphAction(GraphIdx graphIdx) const;
    static QIcon colorIcon(const QColor& color);

    GraphDataModel* _pGraphDataModel;
    QMenu* _pGraphMenu;
    OverlayLabel* _pOverlayLabel;
};

#endif // GRAPHMENUCONTROLLER_H
