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
        VALUE_AXIS,

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
    Qt::DropActions supportedDropActions() const;

    bool removeRows(int row, int count, const QModelIndex &parent);
    bool insertRows(int row, int count, const QModelIndex &parent);

    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    /* Functions for other classes */
    qint32 size() const;
    qint32 activeCount() const;

    GraphData::valueAxis_t valueAxis(quint32 index) const;
    bool isVisible(quint32 index) const;
    QString label(quint32 index) const;
    QColor color(quint32 index) const;
    bool isActive(quint32 index) const;
    QString expression(quint32 index) const;
    GraphData::ExpressionStatus expressionStatus(quint32 index) const;
    qint32 selectedGraph() const;
    QString simplifiedExpression(quint32 index) const;
    QSharedPointer<QCPGraphDataContainer> dataMap(quint32 index);

    qint64 communicationStartTime();
    qint64 communicationEndTime();
    quint32 communicationErrorCount();
    quint32 communicationSuccessCount();
    qint64 communicationRunTime();
    quint32 medianPollTime();

    void setValueAxis(quint32 index, GraphData::valueAxis_t axis);
    void setVisible(quint32 index, bool bVisible);
    void setLabel(quint32 index, const QString &label);
    void setColor(quint32 index, const QColor &color);
    void setActive(quint32 index, bool bActive);
    void setExpression(quint32 index, QString expression);
    void setExpressionStatus(quint32 index, GraphData::ExpressionStatus status);
    void setSelectedGraph(qint32 index);

    void setCommunicationStartTime(qint64 startTime);
    void setCommunicationEndTime(qint64 endTime);
    void setCommunicationStats(quint32 successCount, quint32 errorCount);
    void setMedianPollTime(quint32 pollTime);

    void add(GraphData rowData);
    void add(QList<GraphData> graphDataList);
    void add();
    void add(QList<QString> labelList);
    void setAllData(QList<double> timeData, QList<QList<double> > data);

    void removeRegister(qint32 idx);
    void clear();

    void activeGraphIndexList(QList<quint16> * pList);

    qint32 convertToActiveGraphIndex(quint32 graphIdx);
    qint32 convertToGraphIndex(quint32 activeIdx);

signals:

    void valueAxisChanged(const quint32 graphIdx);
    void visibilityChanged(const quint32 graphIdx);
    void labelChanged(const quint32 graphIdx);
    void colorChanged(const quint32 graphIdx);
    void activeChanged(const quint32 graphIdx);
    void expressionChanged(const quint32 graphIdx);
    void expressionStatusChanged(const quint32 graphIdx);
    void selectedGraphChanged(const qint32 graphIdx);
    void graphsAddData(QList<double>, QList<QList<double> > data);

    void communicationStatsChanged();
    void communicationTimeStatsChanged();

    void moved();
    void added(const quint32 idx);
    void removed(const quint32 idx);

private slots:

    void modelDataChanged(quint32 idx);
    void modelCompleteDataChanged();

private:
    void updateActiveGraphList(void);
    void addToModel(GraphData graphData);
    void removeFromModel(qint32 row);
    void moveRow(int sourceRow, int destRow);

    qint64 _startTime;
    qint64 _endTime;
    quint32 _successCount;
    quint32 _errorCount;
    quint32 _medianPollTime;

    QList<GraphData> _graphData;
    QList<quint32> _activeGraphList;

    qint32 _selectedGraphIdx;

    static const QColor lightRed;
};

#endif // GRAPHDATAMODEL_H
