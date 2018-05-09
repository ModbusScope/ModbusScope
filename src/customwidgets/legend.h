#ifndef LEGEND_H
#define LEGEND_H

#include <QFrame>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QAbstractItemView>
#include <QColor>
#include <QMenu>

/* Forward declaration */
class GuiModel;
class GraphDataModel;
class BasicGraphView;

class Legend : public QFrame
{
    Q_OBJECT
public:
    explicit Legend(QWidget *parent = 0);
    ~Legend();

    void setModels(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);
    void setGraphview(BasicGraphView * pGraphView);
    void graphToForeground(int row);
    void toggleGraphVisibility(int row);

signals:

public slots:
    void addLastReceivedDataToLegend(QList<bool> successList, QList<double> valueList);
    void updateDataInLegend();

private slots:
    void updateLegend();
    void showGraph(quint32 graphIdx);
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphLabel(const quint32 graphIdx);
    void showContextMenu(const QPoint& pos);

    void toggleVisibilityClicked();
    void hideAll();
    void showAll();

private:
    void updateCursorDataInLegend(QStringList &cursorValueList);
    void addItem(quint32 graphIdx);
    void toggleItemVisibility(qint32 activeGraphIdx);

    // Last data
    QStringList _lastReceivedValueList;
    qint32 _popupMenuItem;

    // Models
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    BasicGraphView * _pGraphView;

    // Widgets
    QVBoxLayout * _pLayout;
    QLabel * _pNoGraphs;
    QTableWidget* _pLegendTable;

    QMenu * _pLegendMenu;
    QAction * _pToggleVisibilityAction;
    QAction * _pHideAllAction;
    QAction * _pShowAllAction;

    static const quint32 cColummnColor = 0;
    static const quint32 cColummnValue = 1;
    static const quint32 cColummnText = 2;
};

#endif // LEGEND_H
