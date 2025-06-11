#include "mbcupdatemodel.h"

#include "graphdatamodel.h"

MbcUpdateModel::MbcUpdateModel(GraphDataModel* pGraphDataModel, QObject* parent)
    : QAbstractTableModel(parent), _pGraphDataModel(pGraphDataModel)
{
    _mbcRegisterList.clear();
    _updateInfo = QList<UpdateInfo>(_pGraphDataModel->size());

    connect(_pGraphDataModel, &GraphDataModel::expressionChanged, this, &MbcUpdateModel::checkUpdate);
    connect(_pGraphDataModel, &GraphDataModel::labelChanged, this, &MbcUpdateModel::checkUpdate);
    connect(_pGraphDataModel, &GraphDataModel::added, this, &MbcUpdateModel::checkUpdate);
    connect(_pGraphDataModel, &GraphDataModel::removed, this, &MbcUpdateModel::checkUpdate);
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
    using UpdateField = MbcUpdateModel::UpdateInfo::UpdateField;

    if (!index.isValid())
        return QVariant();

    if (role == Qt::ToolTipRole)
    {
        switch (_updateInfo[index.row()].update)
        {

        case UpdateField::Expression:
            return "Update of expression detected";
            break;

        case UpdateField::Text:
            return "Update of text detected";
            break;

        default:
            return QVariant();
        }
    }
    else if (role == Qt::ForegroundRole)
    {
        return _updateInfo[index.row()].update == UpdateField::None ? QColor(Qt::gray) : QColor(Qt::black);
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
            if (_updateInfo[index.row()].update == UpdateField::Expression)
            {
                return _updateInfo[index.row()].expression;
            }
            break;

        case cColumnUpdateText:
            if (_updateInfo[index.row()].update == UpdateField::Text)
            {
                return _updateInfo[index.row()].text;
            }
            break;

        default:
            return QVariant();
        }
    }
    else if (role == Qt::UserRole)
    {
        const bool textColumnWithUpdate =
          (index.column() == cColumnUpdateText) && (_updateInfo[index.row()].update == UpdateField::Text);
        const bool expressionColumnWithUpdate =
          (index.column() == cColumnUpdateExpression) && (_updateInfo[index.row()].update == UpdateField::Expression);

        if (!textColumnWithUpdate && !expressionColumnWithUpdate)
        {
            return "hidden";
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
    using UpdateField = MbcUpdateModel::UpdateInfo::UpdateField;

    beginResetModel();

    const quint32 graphSize = _pGraphDataModel->size();
    _updateInfo = QList<UpdateInfo>(graphSize);

    QHash<QString, int> exprToIndex;
    QHash<QString, int> labelToIndex;

    // Preprocess graph data model into lookup tables
    for (quint32 idx = 0; idx < graphSize; ++idx)
    {
        exprToIndex.insert(_pGraphDataModel->expression(idx), idx);
        labelToIndex.insert(_pGraphDataModel->label(idx), idx);
    }

    for (const auto& mbcRegister : std::as_const(_mbcRegisterList))
    {
        const QString checkExpr = mbcRegister.toExpression();
        const QString checkName = mbcRegister.name();

        if (exprToIndex.contains(checkExpr))
        {
            int idx = exprToIndex[checkExpr];
            if (_updateInfo[idx].update == UpdateField::None && _pGraphDataModel->label(idx) != checkName)
            {
                _updateInfo[idx].update = UpdateField::Text;
                _updateInfo[idx].expression.clear();
                _updateInfo[idx].text = checkName;
            }
        }
        else if (labelToIndex.contains(checkName))
        {
            int idx = labelToIndex[checkName];
            if (_updateInfo[idx].update == UpdateField::None && _pGraphDataModel->expression(idx) != checkExpr)
            {
                _updateInfo[idx].update = UpdateField::Expression;
                _updateInfo[idx].expression = checkExpr;
                _updateInfo[idx].text.clear();
            }
        }
    }

    endResetModel();
}
