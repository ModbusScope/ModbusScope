#ifndef GRAPHDATASTORE_H
#define GRAPHDATASTORE_H

#include "models/graphdata.h"

#include <QList>
#include <QObject>

class GraphDataStore : public QObject
{
    Q_OBJECT
public:
    explicit GraphDataStore(QObject* parent = nullptr);

    qint32 size() const;
    qint32 activeCount() const;

    GraphData::valueAxis_t valueAxis(quint32 index) const;
    bool isVisible(quint32 index) const;
    QString label(quint32 index) const;
    QColor color(quint32 index) const;
    bool isActive(quint32 index) const;
    QString expression(quint32 index) const;
    GraphData::ExpressionState expressionState(quint32 index) const;
    bool isExpressionValid(quint32 index) const;
    qint32 selectedGraph() const;
    QString simplifiedExpression(quint32 index) const;
    QSharedPointer<const QCPGraphDataContainer> dataMap(quint32 index) const;
    QSharedPointer<QCPGraphDataContainer> mutableDataMap(quint32 index);

    void setValueAxis(quint32 index, GraphData::valueAxis_t axis);
    void setVisible(quint32 index, bool bVisible);
    void setLabel(quint32 index, const QString& label);
    void setColor(quint32 index, const QColor& color);
    void setActive(quint32 index, bool bActive);
    void setExpression(quint32 index, QString expression);
    void setExpressionState(quint32 index, GraphData::ExpressionState status);
    void setSelectedGraph(qint32 index);

    void activeGraphIndexList(QList<quint16>* pList) const;
    qint32 convertToActiveGraphIndex(quint32 graphIdx) const;
    qint32 convertToGraphIndex(quint32 activeIdx) const;

    // Low-level operations used by GraphDataModel to bracket with begin/endInsertRows etc.
    void insertGraphData(GraphData graphData);
    void eraseGraphDataAt(qint32 row);
    void clearAllGraphData();
    void moveGraphRow(int sourceRow, int destRow);

signals:
    void valueAxisChanged(quint32 graphIdx);
    void visibilityChanged(quint32 graphIdx);
    void labelChanged(quint32 graphIdx);
    void colorChanged(quint32 graphIdx);
    void activeChanged(quint32 graphIdx);
    void expressionChanged(quint32 graphIdx);
    void expressionStateChanged(quint32 graphIdx);
    void selectedGraphChanged(qint32 graphIdx);

private:
    void updateActiveGraphList();

    QList<GraphData> _graphData;
    QList<quint32> _activeGraphList;
    qint32 _selectedGraphIdx;
};

#endif // GRAPHDATASTORE_H
