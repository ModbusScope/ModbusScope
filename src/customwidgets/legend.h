#ifndef LEGEND_H
#define LEGEND_H

#include <QFrame>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QAbstractItemView>
#include <QColor>
#include <QMenu>
#include <QTimer>

#include "util/graphindex.h"
#include "util/result.h"

/* Forward declaration */
class GuiModel;
class GraphDataModel;
class GraphView;

class Legend : public QFrame
{
    Q_OBJECT
public:
    explicit Legend(QWidget *parent = nullptr);
    ~Legend();

    void setModels(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);
    void setGraphview(GraphView * pGraphView);

    void clearLegendData();

public slots:
    void addLastReceivedDataToLegend(ResultDoubleList resultList);
    void updateDataInLegend();

private slots:
    void updateLegend();
    void changeGraphVisibility(GraphIdx graphIdx);
    void changeGraphColor(GraphIdx graphIdx);
    void changeGraphAxis(GraphIdx graphIdx);
    void changeGraphLabel(GraphIdx graphIdx);
    void showContextMenu(const QPoint& pos);
    void toggleVisibilityClicked();
    void hideAll();
    void showAll();
    void handleSelectedGraphChanged(GraphIdx graphIdx);

private:
    void updateCursorDataInLegend();
    void updateValueDataInLegend();
    void addItem(GraphIdx graphIdx);
    void toggleItemVisibility(ActiveIdx activeGraphIdx);
    QString valueAxisText(GraphIdx graphIdx);

    void processClick(bool bDoubleClick, int row, int column);
    void cellDoubleClicked(int row, int column);
    void cellClicked(int row, int column);

    // Last data
    ResultDoubleList _lastReceivedList;
    ActiveIdx _popupMenuItem;

    // Models
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    GraphView * _pGraphView;

    QTimer _clickTimer;
    qint32 _clickedRow;
    qint32 _clickedColumn;

    // Widgets
    QVBoxLayout * _pLayout;
    QLabel * _pNoGraphs;
    QTableWidget* _pLegendTable;

    QMenu * _pLegendMenu;
    QAction * _pToggleVisibilityAction;
    QAction * _pHideAllAction;
    QAction * _pShowAllAction;

    static const quint32 cColummnColor = 0;
    static const quint32 cColummnAxis = 1;
    static const quint32 cColummnValue = 2;
    static const quint32 cColummnText = 3;
    static const quint32 cColummnCount = 4;
};

#endif // LEGEND_H
