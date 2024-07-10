#ifndef LEGEND_H
#define LEGEND_H

#include <QFrame>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QAbstractItemView>
#include <QColor>
#include <QMenu>

#include "result.h"

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
    void changeGraphVisibility(quint32 graphIdx);
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphAxis(const quint32 graphIdx);
    void changeGraphLabel(const quint32 graphIdx);
    void showContextMenu(const QPoint& pos);
    void legendCellDoubleClicked(int row, int column);
    void toggleVisibilityClicked();
    void hideAll();
    void showAll();
    void handleSelectedGraphChanged(const qint32 activeGraphIdx);

private:
    void updateCursorDataInLegend();
    void updateValueDataInLegend();
    void addItem(quint32 graphIdx);
    void toggleItemVisibility(qint32 activeGraphIdx);
    QString valueAxisText(quint32 graphIdx);

    // Last data
    ResultDoubleList _lastReceivedList;
    qint32 _popupMenuItem;

    // Models
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    GraphView * _pGraphView;

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
