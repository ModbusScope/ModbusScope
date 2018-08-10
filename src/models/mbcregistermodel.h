#ifndef MBCREGISTERMODEL_H
#define MBCREGISTERMODEL_H

#include <QAbstractTableModel>
#include <mbcregisterdata.h>
#include "graphdatamodel.h"
#include <graphdata.h>

class MbcRegisterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MbcRegisterModel(GraphDataModel * pGraphDataModel, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

    void reset();

    // Add data:
    void fill(QList<MbcRegisterData> mbcRegisterList, QStringList tabList);

    qint32 size() const;

    QList<GraphData> selectedRegisterList();
    quint32 selectedRegisterCount();

    static const quint32 cColumnSelected = 0;
    static const quint32 cColumnAddress = 1;
    static const quint32 cColumnText = 2;
    static const quint32 cColumnUnsigned = 3;
    static const quint32 cColumnTab = 4;
    static const quint32 cColumnCnt = 5;

private:

        struct MbcMetaData
        {
            bool bSelected;
            QString tooltip;
            bool bEnabled;
        };

        bool isAlreadyPresent(QList<MbcRegisterData> mbcRegisterList, quint16 registerAddress);

        QList<MbcRegisterData> _mbcRegisterList;
        QList<struct MbcMetaData> _mbcRegisterMetaDataList;

        QStringList _tabList;

        GraphDataModel * _pGraphDataModel;

};

#endif // MBCREGISTERMODEL_H
