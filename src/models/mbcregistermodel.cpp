#include "mbcregistermodel.h"

#include <QtMath>

MbcRegisterModel::MbcRegisterModel(GraphDataModel *pGraphDataModel, QObject *parent)
    : QAbstractTableModel(parent)
{

    _pGraphDataModel = pGraphDataModel;

    _mbcRegisterList.clear();
    _mbcRegisterMetaDataList.clear();
    _tabList.clear();
}

QVariant MbcRegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case cColumnAddress:
                return QString("Address");
            case cColumnText:
                return QString("Text");
            case cColumnUnsigned:
                return QString("Unsigned");
            case cColumnTab:
                return QString("Tab");
            case cColumnDecimals:
                return QString("Decimals");

            default:
                return QVariant();
            }
        }
        else
        {
            //Can't happen because it is hidden
        }
    }

    return QVariant();
}

int MbcRegisterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return _mbcRegisterList.size();
}

int MbcRegisterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return cColumnCnt;
}

QVariant MbcRegisterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::ToolTipRole)
    {
        return _mbcRegisterMetaDataList[index.row()].tooltip;
    }

    switch (index.column())
    {
    case cColumnSelected:
        if (role == Qt::CheckStateRole)
        {
            if (_mbcRegisterMetaDataList[index.row()].bSelected)
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;

    case cColumnAddress:
        if (role == Qt::DisplayRole)
        {
            return _mbcRegisterList[index.row()].registerAddress();
        }
        break;

    case cColumnDecimals:
        if (role == Qt::DisplayRole)
        {
            return _mbcRegisterList[index.row()].decimals();
        }
        break;

    case cColumnTab:
        {
            const qint32 tabIdx = _mbcRegisterList[index.row()].tabIdx();

            if (tabIdx < _tabList.size())
            {
                if (role == Qt::DisplayRole)
                {
                    return _tabList[tabIdx];
                }
            }
        }
        break;

    case cColumnText:
        if (role == Qt::DisplayRole)
        {
            return _mbcRegisterList[index.row()].name();
        }
        break;

    case cColumnUnsigned:
        if (role == Qt::CheckStateRole)
        {
            if (_mbcRegisterList[index.row()].isUnsigned())
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    default:
        return QVariant();
    }

    return QVariant();
}

bool MbcRegisterModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (!index.isValid())
        return false;

    bool bRet = false;

    switch (index.column())
    {
    case cColumnSelected:
        if (role == Qt::CheckStateRole)
        {
            _mbcRegisterMetaDataList[index.row()].bSelected = value == Qt::Checked;

            updateAlreadySelected();

            bRet = true;
        }
        break;


    default:
        break;
    }

    if (bRet)
    {
        // Notify view(s) of change
        emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0));
    }

    return bRet;
}

void MbcRegisterModel::reset()
{
    beginResetModel();

    _mbcRegisterList.clear();
    _mbcRegisterMetaDataList.clear();
    _tabList.clear();

    endResetModel();
}

void MbcRegisterModel::fill(QList<MbcRegisterData> mbcRegisterList, QStringList tabList)
{
    if (rowCount() != 0)
    {
        reset();
    }

    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + mbcRegisterList.size() - 1);

    _tabList = tabList;

    for(qint32 idx = 0; idx < mbcRegisterList.size(); idx++)
    {
        // Get result before adding to list
        const uint32_t bitmask = 0xFFFFFFFF;

        _mbcRegisterList.append(mbcRegisterList[idx]);

        _mbcRegisterMetaDataList.append( {false, QString(""), false, false} );

        /* Disable all duplicates */
        if (!_mbcRegisterList.last().isReadable())
        {
            _mbcRegisterMetaDataList.last().tooltip = tr("Not readable");
        }
        else if (_pGraphDataModel->isPresent(mbcRegisterList[idx].registerAddress(), bitmask))
        {
            _mbcRegisterMetaDataList.last().tooltip = tr("Already added address");
        }
        else
        {
            _mbcRegisterMetaDataList.last().bEnabled = true;
        }
    }

    updateAlreadySelected();

    /* Call function to trigger view update */
    endInsertRows();
}

Qt::ItemFlags MbcRegisterModel::flags(const QModelIndex & index) const
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
    else if (index.column() == cColumnUnsigned)
    {
        // checkable
        flags = Qt::ItemIsUserCheckable;
    }

    if (_mbcRegisterMetaDataList[index.row()].bEnabled  && !_mbcRegisterMetaDataList[index.row()].bAlreadyStaged)
    {
        flags |= Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
    }

    return flags;
}

QList<GraphData> MbcRegisterModel::selectedRegisterList()
{
    QList<GraphData> _selectedRegisterList;

    // Get selected register from table widget */
    for (qint32 row = 0; row < _mbcRegisterMetaDataList.size(); row++)
    {
        if (_mbcRegisterMetaDataList[row].bSelected)
        {
            GraphData graphData;
            _mbcRegisterList[row];

            graphData.setActive(true);
            graphData.setRegisterAddress(_mbcRegisterList[row].registerAddress());
            graphData.setLabel(_mbcRegisterList[row].name());
            graphData.setUnsigned(_mbcRegisterList[row].isUnsigned());
            graphData.setDivideFactor(static_cast<double>(qPow(10, _mbcRegisterList[row].decimals())));
            graphData.setBit32(_mbcRegisterList[row].is32Bit());

            _selectedRegisterList.append(graphData);
        }
    }

    return _selectedRegisterList;
}

quint32 MbcRegisterModel::selectedRegisterCount()
{
    quint32 cnt = 0;

    // Get selected register from table widget */
    for (qint32 row = 0; row < _mbcRegisterMetaDataList.size(); row++)
    {
        if (_mbcRegisterMetaDataList[row].bSelected)
        {
            cnt++;
        }
    }

    return cnt;
}

void MbcRegisterModel::updateAlreadySelected()
{
    for (int32_t idx = 0; idx < _mbcRegisterMetaDataList.size(); idx++)
    {
        bool bFound = false;

        for (int32_t idxSelected = 0; idxSelected < _mbcRegisterMetaDataList.size(); idxSelected++)
        {
            if (_mbcRegisterMetaDataList[idxSelected].bSelected)
            {

                if (idx != idxSelected)
                {
                    if (_mbcRegisterList[idx].registerAddress() == _mbcRegisterList[idxSelected].registerAddress())
                    {
                        bFound = true;
                        break;
                    }
                }
                else
                {
                    /* Skip same id's */
                }
            }
        }

        if (_mbcRegisterMetaDataList[idx].bEnabled)
        {
            /* Mark index as already selected (or not) */
            if (bFound && !_mbcRegisterMetaDataList[idx].bSelected)
            {
                _mbcRegisterMetaDataList[idx].bAlreadyStaged = true;
                _mbcRegisterMetaDataList[idx].tooltip = tr("Already selected address");
            }
            else
            {
                _mbcRegisterMetaDataList[idx].bAlreadyStaged = false;
                _mbcRegisterMetaDataList[idx].tooltip = tr("");
            }
        }
    }
}
