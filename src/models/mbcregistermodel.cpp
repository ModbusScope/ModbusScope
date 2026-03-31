#include "mbcregistermodel.h"

#include "customwidgets/centeredbox.h"

/*!
 * \brief Constructs an empty MbcRegisterModel.
 */
MbcRegisterModel::MbcRegisterModel(QObject* parent) : QAbstractTableModel(parent)
{
    _mbcRegisterList.clear();
    _tabList.clear();
    _selection = Qt::Unchecked;
}

/*!
 * \brief Returns header data for the given section.
 *
 * Section 0 uses Qt::CheckStateRole to drive the select-all checkbox in MbcHeader.
 */
QVariant MbcRegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case cColumnAddress:
                return QString("Address");
            case cColumnText:
                return QString("Text");
            case cColumnType:
                return QString("Type");
            case cColumnTab:
                return QString("Tab");
            case cColumnDecimals:
                return QString("Decimals");

            default:
                return QVariant();
            }
        }
        else if (role == Qt::CheckStateRole)
        {
            if (section == cColumnSelected)
            {
                return _selection;
            }
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        // Can't happen because it is hidden
    }

    return QVariant();
}

/*!
 * \brief Sets header data; used to drive the select-all checkbox state.
 * \return True when the change was applied.
 */
bool MbcRegisterModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    bool bRet = false;

    if ((section == cColumnSelected) && (role == Qt::CheckStateRole))
    {
        auto selectAllState = static_cast<Qt::CheckState>(value.toUInt());
        if (_selection <= Qt::Checked)
        {
            _selection = selectAllState;
            bRet = true;
        }
    }

    if (bRet)
    {
        // Notify view(s) of change
        emit headerDataChanged(orientation, cColumnSelected, cColumnSelected);
    }

    return bRet;
}

/*!
 * \brief Returns the number of rows (registers) in the model.
 */
int MbcRegisterModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return _mbcRegisterList.size();
}

/*!
 * \brief Returns the number of columns.
 */
int MbcRegisterModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return cColumnCnt;
}

/*!
 * \brief Returns data for the given index and role.
 */
QVariant MbcRegisterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto const mbcRegister = _mbcRegisterList[index.row()];

    if (role == Qt::ToolTipRole)
    {
        if (!mbcRegister.registerData.isReadable())
        {
            return "Not readable";
        }
        else
        {
            return "";
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (index.column() == cColumnSelected)
        {
            if (mbcRegister.bSelected)
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
    }
    else if (role == CheckAlignmentRole)
    {
        if (index.column() == cColumnSelected)
        {
            return Qt::AlignCenter;
        }
    }
    else if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {

        case cColumnAddress:
            return mbcRegister.registerData.registerAddress();
            break;

        case cColumnDecimals:
            return mbcRegister.registerData.decimals();
            break;

        case cColumnTab:
        {
            const qint32 tabIdx = mbcRegister.registerData.tabIdx();

            if (tabIdx < _tabList.size())
            {
                return _tabList[tabIdx];
            }
        }
        break;

        case cColumnText:
            return mbcRegister.registerData.name();
            break;

        case cColumnType:
            return ModbusDataType::typeString(mbcRegister.registerData.type());
            break;

        default:
            return QVariant();
        }
    }

    return QVariant();
}

/*!
 * \brief Sets check state for cColumnSelected when the item is enabled.
 * \return True when the change was applied.
 */
bool MbcRegisterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    bool bRet = false;

    if ((index.column() == cColumnSelected) && (role == Qt::CheckStateRole) && _mbcRegisterList[index.row()].bEnabled)
    {
        _mbcRegisterList[index.row()].bSelected = value == Qt::Checked;

        bRet = true;
    }

    if (bRet)
    {
        emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0));
    }

    return bRet;
}

/*!
 * \brief Sets the check state for the given list of indices.
 *
 * Non-enabled rows are always forced to unchecked.
 */
void MbcRegisterModel::setSelectionstate(QList<QModelIndex>& indexList, Qt::CheckState state)
{
    if (indexList.isEmpty())
    {
        return;
    }

    for (QModelIndex index : indexList)
    {
        if (index.isValid())
        {
            if (_mbcRegisterList[index.row()].bEnabled)
            {
                _mbcRegisterList[index.row()].bSelected = state == Qt::Checked;
            }
            else
            {
                _mbcRegisterList[index.row()].bSelected = Qt::Unchecked;
            }
        }
    }

    emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0));
}

/*!
 * \brief Clears all data from the model.
 */
void MbcRegisterModel::reset()
{
    beginResetModel();

    _mbcRegisterList.clear();
    _tabList.clear();
    _selection = Qt::Unchecked;

    endResetModel();
}

/*!
 * \brief Fills the model with a list of parsed register definitions.
 * \param mbcRegisterList Registers read from the MBC file.
 * \param tabList Tab names corresponding to the tabIdx fields.
 */
void MbcRegisterModel::fill(QList<MbcRegisterData> mbcRegisterList, QStringList tabList)
{
    if (rowCount() != 0)
    {
        reset();
    }

    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + mbcRegisterList.size() - 1);

    _tabList = tabList;

    for (qint32 idx = 0; idx < mbcRegisterList.size(); idx++)
    {
        // Get result before adding to list
        _mbcRegisterList.append({ mbcRegisterList[idx], false, false });

        if (_mbcRegisterList.last().registerData.isReadable())
        {
            _mbcRegisterList.last().bEnabled = true;
        }
    }

    /* Call function to trigger view update */
    endInsertRows();
}

/*!
 * \brief Returns item flags for the given index.
 *
 * Only cColumnSelected is user-checkable. Non-readable rows are disabled.
 */
Qt::ItemFlags MbcRegisterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::NoItemFlags;

    if (index.column() == cColumnSelected)
    {
        // checkable
        flags = Qt::ItemIsUserCheckable;
    }

    if (_mbcRegisterList[index.row()].bEnabled)
    {
        flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return flags;
}

/*!
 * \brief Returns a list of GraphData objects for all selected registers.
 */
QList<GraphData> MbcRegisterModel::selectedRegisterList()
{
    QList<GraphData> _selectedRegisterList;

    // Get selected register from table widget */
    for (const MbcRegister& row : std::as_const(_mbcRegisterList))
    {
        if (row.bSelected)
        {
            GraphData graphData;
            graphData.setActive(true);
            graphData.setLabel(row.registerData.name());
            graphData.setExpression(row.registerData.toExpression());

            _selectedRegisterList.append(graphData);
        }
    }

    return _selectedRegisterList;
}

/*!
 * \brief Returns the count of currently selected registers.
 */
quint32 MbcRegisterModel::selectedRegisterCount()
{
    quint32 cnt = 0;

    // Get selected register from table widget */
    for (const MbcRegister& row : std::as_const(_mbcRegisterList))
    {
        if (row.bSelected)
        {
            cnt++;
        }
    }

    return cnt;
}

/*!
 * \brief Unchecks every row in the source model regardless of current filter state.
 */
void MbcRegisterModel::clearAllSelections()
{
    for (MbcRegister& row : _mbcRegisterList)
    {
        row.bSelected = false;
    }

    if (!_mbcRegisterList.isEmpty())
    {
        emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0));
    }
}
