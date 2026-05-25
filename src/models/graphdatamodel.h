#ifndef GRAPHDATAMODEL_H
#define GRAPHDATAMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QObject>

#include "models/graphdata.h"
#include "util/graphindex.h"

class GraphDataStore;

class GraphDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum column
    {
        COLOR = 0,
        ACTIVE,
        TEXT,
        EXPRESSION,
        VALUE_AXIS,

        COUNT
    };

    static const QColor lightRed;

    explicit GraphDataModel(QObject* parent = nullptr);

    /* Functions for QTableView (model) */
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    Qt::ItemFlags flags(const QModelIndex& index) const;
    Qt::DropActions supportedDropActions() const;

    bool removeRows(int row, int count, const QModelIndex& parent);
    bool insertRows(int row, int count, const QModelIndex& parent);

    QMimeData* mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    /* Functions for other classes */
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
    QSharedPointer<const QCPGraphDataContainer> dataMap(GraphIdx index) const;
    QSharedPointer<QCPGraphDataContainer> mutableDataMap(GraphIdx index);

    void setValueAxis(GraphIdx index, GraphData::valueAxis_t axis);
    void setVisible(GraphIdx index, bool bVisible);
    void setLabel(GraphIdx index, const QString& label);
    void setColor(GraphIdx index, const QColor& color);
    void setActive(GraphIdx index, bool bActive);
    void setExpression(GraphIdx index, QString expression);
    void setExpressionState(GraphIdx index, GraphData::ExpressionState status);
    void setSelectedGraph(GraphIdx index);

    void setDefaultExpression(const QString& expression);

    void add(GraphData rowData);
    void add(QList<GraphData> graphDataList);
    void add();
    void add(QList<QString> labelList);
    void setAllData(QList<double> timeData, QList<QList<double> > data);

    void removeRegister(GraphIdx idx);
    void clear();

    void activeGraphIndexList(QList<GraphIdx>& list) const;

    ActiveIdx convertToActiveGraphIndex(GraphIdx graphIdx) const;
    GraphIdx convertToGraphIndex(ActiveIdx activeIdx) const;

signals:

    void valueAxisChanged(GraphIdx graphIdx);
    void visibilityChanged(GraphIdx graphIdx);
    void labelChanged(GraphIdx graphIdx);
    void colorChanged(GraphIdx graphIdx);
    void activeChanged(GraphIdx graphIdx);
    void expressionChanged(GraphIdx graphIdx);
    void expressionStateChanged(GraphIdx graphIdx);
    void selectedGraphChanged(GraphIdx graphIdx);
    void graphsAddData(QList<double>, QList<QList<double> > data);

    void moved();
    void added(GraphIdx idx);
    void removed(GraphIdx idx);

private slots:

    void modelDataChanged(GraphIdx idx);
    void modelCompleteDataChanged();

private:
    void addToModel(GraphData graphData);
    void removeFromModel(int row);
    void moveRow(int sourceRow, int destRow);

    QString _defaultExpression;

    GraphDataStore* _pStore;
};

#endif // GRAPHDATAMODEL_H
