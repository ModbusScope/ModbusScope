#ifndef LEGEND_H
#define LEGEND_H

#include <QFrame>
#include <QVBoxLayout>

#include "legenditem.h"

/* Forward declaration */
class GuiModel;
class GraphDataModel;
class BasicGraphView;

class Legend : public QFrame
{
    Q_OBJECT
public:
    explicit Legend(QWidget *parent = 0);

    void setModels(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel);
    void setGraphview(BasicGraphView * pGraphView);

    void mousePressEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);

signals:

public slots:
    void addLastReceivedDataToLegend(QList<bool> successList, QList<double> valueList);
    void updateDataInLegend();

private slots:
    void updateLegend();
    void showGraph(quint32 graphIdx);
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphLabel(const quint32 graphIdx);

private:
    void updateCursorDataInLegend(QStringList &cursorValueList);
    void addItem(quint32 graphIdx);
    qint32 itemUnderCursor();

    // Last data
    QStringList _lastReceivedValueList;

    // Models
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    BasicGraphView * _pGraphView;

    // Widgets
    QVBoxLayout * _pLayout;
    QLabel * _pNoGraphs;
    QList<LegendItem *> _items;
};

#endif // LEGEND_H
