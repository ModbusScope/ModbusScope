#ifndef MBCREGISTERMODEL_H
#define MBCREGISTERMODEL_H

#include "importexport/mbcregisterdata.h"
#include "models/graphdata.h"

#include <QAbstractTableModel>

class MbcRegisterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MbcRegisterModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section,
                       Qt::Orientation orientation,
                       const QVariant& value,
                       int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

    void setSelectionstate(QList<QModelIndex>& indexList, Qt::CheckState state);

    void reset();

    // Add data:
    void fill(QList<MbcRegisterData> mbcRegisterList, QStringList tabList);

    QList<GraphData> selectedRegisterList();
    quint32 selectedRegisterCount();

    static const quint32 cColumnSelected = 0;
    static const quint32 cColumnAddress = 1;
    static const quint32 cColumnText = 2;
    static const quint32 cColumnType = 3;
    static const quint32 cColumnTab = 4;
    static const quint32 cColumnDecimals = 5;
    static const quint32 cColumnCnt = 6;

private:
    typedef struct
    {
        MbcRegisterData registerData;
        bool bSelected;
        bool bEnabled;
    } MbcRegister;

    QList<MbcRegister> _mbcRegisterList;

    QStringList _tabList;

    Qt::CheckState _selection;
};

#endif // MBCREGISTERMODEL_H
