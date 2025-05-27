#include "mbcupdatemodel.h"

#include "graphdatamodel.h"

#include <algorithm>

MbcUpdateModel::MbcUpdateModel(GraphDataModel* pGraphDataModel, QObject* parent)
    : QAbstractTableModel(parent), _pGraphDataModel(pGraphDataModel)
{
    _mbcRegisterList.clear();
    _updateInfo = QList<UpdateInfo>(_pGraphDataModel->size());
}

QVariant MbcUpdateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
        case cColumnExpression:
            return QString("Expression");
        case cColumnText:
            return QString("Text");
        case cColumnUpdateExpression:
            return QString("Update Expression");
        case cColumnUpdateText:
            return QString("Update Text");
        default:
            return QVariant();
        }
    }
    else
    {
        // Can't happen because it is hidden
    }

    return QVariant();
}

int MbcUpdateModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return _pGraphDataModel->size();
}

int MbcUpdateModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return cColumnCnt;
}

Qt::ItemFlags MbcUpdateModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::NoItemFlags;
    flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    return flags;
}

QVariant MbcUpdateModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::ToolTipRole)
    {
        // TODO
        return "test";
    }
    else if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {

        case cColumnExpression:
            return _pGraphDataModel->simplifiedExpression(index.row());
            break;

        case cColumnText:
            return _pGraphDataModel->label(index.row());
            break;

        case cColumnUpdateExpression:
            if (_updateInfo[index.row()].bUpdate)
            {
                return _updateInfo[index.row()].expression;
            }
            break;

        case cColumnUpdateText:
            if (_updateInfo[index.row()].bUpdate)
            {
                return _updateInfo[index.row()].text;
            }
            break;

        default:
            return QVariant();
        }
    }

    return QVariant();
}

void MbcUpdateModel::setMbcRegisters(QList<MbcRegisterData> mbcRegisterList)
{
    _mbcRegisterList = mbcRegisterList;

    checkUpdate();
}

void MbcUpdateModel::checkUpdate()
{
    const quint32 graphSize = std::max(_pGraphDataModel->size(), 0);

    _updateInfo = QList<UpdateInfo>(graphSize);

    for (const auto& mbcRegister : std::as_const(_mbcRegisterList))
    {
        auto const checkExpr = mbcRegister.toExpression();
        auto const checkName = mbcRegister.name();

        for (quint32 idx = 0; idx < graphSize; idx++)
        {
            QString const label = _pGraphDataModel->label(idx);
            QString const expr = _pGraphDataModel->expression(idx);

            if (!_updateInfo[idx].bUpdate)
            {
                if ((checkExpr == expr) && (checkName != label))
                {
                    _updateInfo[idx].bUpdate = true;
                    _updateInfo[idx].expression = QString();
                    _updateInfo[idx].text = checkName;
                    break;
                }
                else if ((checkExpr != expr) && (checkName == label))
                {
                    _updateInfo[idx].bUpdate = true;
                    _updateInfo[idx].expression = checkExpr;
                    _updateInfo[idx].text = QString();
                    break;
                }
                else
                {
                    // no update
                }
            }
        }
    }
}
