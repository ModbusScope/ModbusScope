#ifndef TST_GRAPHMENUCONTROLLER_H
#define TST_GRAPHMENUCONTROLLER_H

#include <QObject>

class GraphDataModel;
class OverlayLabel;
class GraphMenuController;
class QMenu;
class QWidget;

class TestGraphMenuController : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void emptyModelDisablesMenu();
    void addGraphsBuildsMenu();
    void labelChangeUpdatesAction();
    void colorChangeUpdatesIcon();
    void visibilityChangeChecksAction();
    void menuClickTogglesModelVisibility();
    void inactiveGraphIgnored();

private:
    GraphDataModel* _pGraphDataModel = nullptr;
    QWidget* _pHost = nullptr;
    QMenu* _pMenu = nullptr;
    OverlayLabel* _pOverlayLabel = nullptr;
    GraphMenuController* _pController = nullptr;
};

#endif // TST_GRAPHMENUCONTROLLER_H
