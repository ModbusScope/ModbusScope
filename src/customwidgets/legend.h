#ifndef LEGEND_H
#define LEGEND_H

#include <QFrame>
#include <QVBoxLayout>

#include "legenditem.h"

/* Forward declaration */
class GuiModel;
class GraphDataModel;

class Legend : public QFrame
{
    Q_OBJECT
public:
    explicit Legend(QWidget *parent = 0);

    void setModels(GuiModel *pGuiModel, GraphDataModel * pGraphDataModel);

    void mousePressEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);

signals:

public slots:
    void addDataToLegend(QList<bool> successList, QList<double> valueList);

private slots:
    void updateLegend();
    void showGraph(quint32 graphIdx);
    void changeGraphColor(const quint32 graphIdx);
    void changeGraphLabel(const quint32 graphIdx);

private:

    void addItem(quint32 graphIdx);
    qint32 itemUnderCursor();


    // Models
    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    // Widgets
    QVBoxLayout * _pLayout;
    QLabel * _pNoGraphs;
    QList<LegendItem *> _items;
};

#endif // LEGEND_H
