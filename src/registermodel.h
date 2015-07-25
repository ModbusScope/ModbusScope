#ifndef REGISTERMODEL_H
#define REGISTERMODEL_H

#include <QAbstractTableModel>
#include "communicationmanager.h"
#include "registerdata.h"

class RegisterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RegisterModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);

    uint checkedRegisterCount();
    void registerList(QList<quint16> * pRegisterList);
    void checkedRegisterList(QList<RegisterData> *pRegisterList);

    void clear(const QModelIndex &parent = QModelIndex());
    void appendRow(RegisterData rowData, const QModelIndex &parent = QModelIndex());

    bool areExclusive(quint16 * pRegister);

signals:

public slots:

private:
    quint16 nextFreeAddress();

    QList<RegisterData> _dataList;

};

#endif // REGISTERMODEL_H
