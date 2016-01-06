#ifndef REGISTERMODEL_H
#define REGISTERMODEL_H

#include <QAbstractTableModel>
#include "communicationmanager.h"
#include "graphdatamodel.h"

class RegisterDialogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RegisterDialogModel(GraphDataModel * pGraphDataModel, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);

signals:

public slots:

private slots:
    void modelDataChanged(qint32 idx);
    void modelDataChanged(quint32 idx);
    void modelDataChanged();

private:

    GraphDataModel * _pGraphDataModel;

};

#endif // REGISTERMODEL_H
