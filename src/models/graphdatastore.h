#ifndef GRAPHDATASTORE_H
#define GRAPHDATASTORE_H

#include "models/graphdata.h"
#include "util/graphindex.h"

#include <QList>
#include <QObject>

class GraphDataStore : public QObject
{
    Q_OBJECT
public:
    explicit GraphDataStore(QObject* parent = nullptr);

    qint32 size() const;
    qint32 activeCount() const;

    GraphData::valueAxis_t valueAxis(GraphIdx index) const;
    bool isVisible(GraphIdx index) const;
    QString label(GraphIdx index) const;
    QColor color(GraphIdx index) const;
    bool isActive(GraphIdx index) const;
    QString expression(GraphIdx index) const;
    GraphData::ExpressionState expressionState(GraphIdx index) const;
    bool isExpressionValid(GraphIdx index) const;
    GraphIdx selectedGraph() const;
    QString simplifiedExpression(GraphIdx index) const;
    QSharedPointer<const GraphDataSeries> dataSeries(GraphIdx index) const;
    QSharedPointer<GraphDataSeries> mutableDataSeries(GraphIdx index);

    void setValueAxis(GraphIdx index, GraphData::valueAxis_t axis);
    void setVisible(GraphIdx index, bool bVisible);
    void setLabel(GraphIdx index, const QString& label);
    void setColor(GraphIdx index, const QColor& color);
    void setActive(GraphIdx index, bool bActive);
    void setExpression(GraphIdx index, QString expression);
    void setExpressionState(GraphIdx index, GraphData::ExpressionState status);
    void setSelectedGraph(GraphIdx index);

    void activeGraphIndexList(QList<GraphIdx>& list) const;
    ActiveIdx convertToActiveGraphIndex(GraphIdx graphIdx) const;
    GraphIdx convertToGraphIndex(ActiveIdx activeIdx) const;

    // Low-level operations used by GraphDataModel to bracket with begin/endInsertRows etc.
    void insertGraphData(GraphData graphData);
    void eraseGraphDataAt(GraphIdx row);
    void clearAllGraphData();
    void moveGraphRow(GraphIdx sourceRow, GraphIdx destRow);

signals:
    void valueAxisChanged(GraphIdx graphIdx);
    void visibilityChanged(GraphIdx graphIdx);
    void labelChanged(GraphIdx graphIdx);
    void colorChanged(GraphIdx graphIdx);
    void activeChanged(GraphIdx graphIdx);
    void expressionChanged(GraphIdx graphIdx);
    void expressionStateChanged(GraphIdx graphIdx);
    void selectedGraphChanged(GraphIdx graphIdx);

private:
    void resetSelection();
    void updateActiveGraphList();

    QList<GraphData> _graphData;
    QList<GraphIdx> _activeGraphList;
    GraphIdx _selectedGraphIdx;
};

#endif // GRAPHDATASTORE_H
