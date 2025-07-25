#ifndef MBCUPDATEMODEL_H
#define MBCUPDATEMODEL_H

#include "importexport/mbcregisterdata.h"

#include <QAbstractTableModel>

// Forward declaration
class GraphDataModel;

class MbcUpdateModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MbcUpdateModel(GraphDataModel* pGraphDataModel, QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setMbcRegisters(QList<MbcRegisterData> mbcRegisterList);

    static const quint32 cColumnExpression = 0;
    static const quint32 cColumnText = 1;
    static const quint32 cColumnUpdateExpression = 2;
    static const quint32 cColumnUpdateText = 3;
    static const quint32 cColumnCnt = 4;

private slots:
    void checkUpdate();

private:
    class UpdateInfo
    {

    public:
        enum class UpdateField
        {
            None,
            Text,
            Expression
        };
        UpdateField update = UpdateField::None;
        QString text;
        QString expression;
    };

    GraphDataModel* _pGraphDataModel;

    QList<MbcRegisterData> _mbcRegisterList;
    QList<UpdateInfo> _updateInfo;
};

#endif // MBCUPDATEMODEL_H
