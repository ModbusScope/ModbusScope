#ifndef GRAPHDATAMODEL_H
#define GRAPHDATAMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QList>

#include "graphdata.h"


class GraphDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum column {
        COLOR = 0,
        ACTIVE,
        TEXT,
        EXPRESSION,

        COUNT
    };

    explicit GraphDataModel(QObject *parent = nullptr);

    /* Functions for QTableView (model) */
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);

    /* Functions for other classes */
    qint32 size() const;
    qint32 activeCount() const;

    bool isVisible(quint32 index) const;
    QString label(quint32 index) const;
    QColor color(quint32 index) const;
    bool isActive(quint32 index) const;
    QString expression(quint32 index) const;
    QSharedPointer<QCPGraphDataContainer> dataMap(quint32 index);

    void setVisible(quint32 index, bool bVisible);
    void setLabel(quint32 index, const QString &label);
    void setColor(quint32 index, const QColor &color);
    void setActive(quint32 index, bool bActive);
    void setExpression(quint32 index, QString expression);

    void add(GraphData rowData);
    void add(QList<GraphData> graphDataList);
    void add();
    void add(QList<QString> labelList, QList<double> timeData, QList<QList<double> > data);

    void removeRegister(qint32 idx);
    void clear();

    void activeGraphIndexList(QList<quint16> * pList);

    qint32 convertToActiveGraphIndex(quint32 graphIdx);
    qint32 convertToGraphIndex(quint32 activeIdx);

signals:

    void visibilityChanged(const quint32 graphIdx);
    void labelChanged(const quint32 graphIdx);
    void colorChanged(const quint32 graphIdx);
    void activeChanged(const quint32 graphIdx); // when graph is actived / deactivated
    void expressionChanged(const quint32 graphIdx);
    void graphsAddData(QList<double>, QList<QList<double> > data);

    void added(const quint32 idx); // When graph definition is added
    void removed(const quint32 idx); // When graph definition is removed

private slots:

    void modelDataChanged(quint32 idx);
    void modelCompleteDataChanged();

private:
    void updateActiveGraphList(void);
    void addToModel(GraphData * pGraphData);
    void removeFromModel(qint32 row);

    QList<GraphData> _graphData;
    QList<quint32> _activeGraphList;
};

#endif // GRAPHDATAMODEL_H
