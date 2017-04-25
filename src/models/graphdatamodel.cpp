
#include <QMessageBox>
#include "graphdata.h"
#include "util.h"
#include "QDebug"

#include "graphdatamodel.h"


GraphDataModel::GraphDataModel(QObject *parent) : QAbstractTableModel(parent)
{
    _graphData.clear();

    connect(this, SIGNAL(visibilityChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(labelChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(colorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(activeChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(unsignedChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(multiplyFactorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(divideFactorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(registerAddressChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(bitmaskChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(shiftChanged(quint32)), this, SLOT(modelDataChanged(quint32)));

    connect(this, SIGNAL(added(quint32)), this, SLOT(modelDataChanged()));
    connect(this, SIGNAL(removed(quint32)), this, SLOT(modelDataChanged()));
}

int GraphDataModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

int GraphDataModel::columnCount(const QModelIndex & /*parent*/) const
{
    /*
    * color
    * bActive
    * bUnsigned
    * Register
    * Text
    * Bitmask
    * Shift
    * Multiply factor
    * Divide factor
    * */
    return 9; // Number of visible members of struct
}

QVariant GraphDataModel::data(const QModelIndex &index, int role) const
{

    switch (index.column())
    {
    case 0:
        if (role == Qt::BackgroundColorRole)
        {
            return color(index.row());
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (isActive(index.row()))
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case 2:
        if (role == Qt::CheckStateRole)
        {
            if (isUnsigned(index.row()))
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case 3:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return registerAddress(index.row());
        }
        break;
    case 4:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return label(index.row());
        }
        break;
    case 5:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            // Show hex value
            return QString("0x%1").arg(bitmask(index.row()), 0, 16);
        }
        break;
    case 6:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return shift(index.row());
        }
        break;
    case 7:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(multiplyFactor(index.row()));
        }
        break;
    case 8:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(divideFactor(index.row()));
        }
        break;
    default:
        return QVariant();
        break;

    }

    return QVariant();
}

QVariant GraphDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return QString("Color");
            case 1:
                return QString("Active");
            case 2:
                return QString("Unsigned");
            case 3:
                return QString("Address");
            case 4:
                return QString("Text");
            case 5:
                return QString("Bitmask");
            case 6:
                return QString("Shift");
            case 7:
                return QString("Multiply");
            case 8:
                return QString("Divide");
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


bool GraphDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case 0:
        if (role == Qt::EditRole)
        {
            if (value.canConvert(QMetaType::QColor))
            {
                QColor color = value.value<QColor>();
                setColor(index.row(), color);
            }
            else
            {
                bRet = false;
            }
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setActive(index.row(), true);
            }
            else
            {
                setActive(index.row(), false);
            }
        }
        break;
    case 2:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setUnsigned(index.row(), true);
            }
            else
            {
                setUnsigned(index.row(), false);
            }
        }
        break;
    case 3:
        if (role == Qt::EditRole)
        {
            bool bOk = false;

            if (value.canConvert(QMetaType::UInt))
            {
                 const quint32 newAddr = value.toUInt();
                 if (
                         (newAddr >= 40001)
                         && (newAddr <= 49999)
                    )
                 {
                     bOk = true;
                     setRegisterAddress(index.row(), (quint16)newAddr);
                 }
            }

            if (!bOk)
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Register address is not a valid address between 40001 and 49999."));
                msgBox.exec();
                bRet = false;
            }
        }
        break;
    case 4:
        if (role == Qt::EditRole)
        {
            setLabel(index.row(), value.toString());
        }
        break;
    case 5:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint16 newBitMask = value.toString().toUInt(&bSuccess, 0);

            if (bSuccess)
            {
                setBitmask(index.row(), newBitMask);
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Bitmask is not a valid integer."));
                msgBox.exec();
            }
        }
        break;
    case 6:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const qint32 newShift = value.toString().toInt(&bSuccess);

            if (
                    (bSuccess)
                    &&
                    (
                        (newShift > -16)
                        && (newShift < 16)
                    )
                )
            {
                setShift(index.row(), newShift);
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Shift is not a valid integer between -16 and 16."));
                msgBox.exec();
                break;
            }
        }
        break;
    case 7:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const double parseResult = QLocale::system().toDouble(value.toString(), &bSuccess);
            if (bSuccess)
            {
                setMultiplyFactor(index.row(), parseResult);
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Multiply factor is not a valid double. Did you use correct decimal separator character? Expecting \"%1\"").arg(QLocale::system().decimalPoint()));
                msgBox.exec();
                break;
            }
        }
        break;
    case 8:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const double parseResult = QLocale::system().toDouble(value.toString(), &bSuccess);
            if (bSuccess)
            {
                setDivideFactor(index.row(), parseResult);
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Divide factor is not a valid double. Did you use correct decimal separator character? Expecting \"%1\"").arg(QLocale::system().decimalPoint()));
                msgBox.exec();
                break;
            }
        }
        break;
    default:
        break;

    }

    // Notify view(s) of change
    emit dataChanged(index, index);

    return bRet;
}

Qt::ItemFlags GraphDataModel::flags(const QModelIndex & index) const
{
    if (
            (index.column() == 1)
            || (index.column() == 2)
        )
    {
        // checkable
        return Qt::ItemIsSelectable |  Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    }
    else if (index.column() == 0)
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
    }
    else
    {
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

}


bool GraphDataModel::removeRows (int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    removeFromModel(row);

    return true;
}

bool GraphDataModel::insertRows (int row, int count, const QModelIndex &parent)
{
    if (
        (count != 1)
        || (row != size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);

    add();

    return true;
}

qint32 GraphDataModel::size() const
{
    return _graphData.size();
}

qint32 GraphDataModel::activeCount() const
{
    return _activeGraphList.size();
}

bool GraphDataModel::isVisible(quint32 index) const
{
    return _graphData[index].isVisible();
}

QString GraphDataModel::label(quint32 index) const
{
    return _graphData[index].label();
}

QColor GraphDataModel::color(quint32 index) const
{
    return _graphData[index].color();
}

bool GraphDataModel::isActive(quint32 index) const
{
    return _graphData[index].isActive();
}

bool GraphDataModel::isUnsigned(quint32 index) const
{
    return _graphData[index].isUnsigned();
}

double GraphDataModel::multiplyFactor(quint32 index) const
{
    return _graphData[index].multiplyFactor();
}

double GraphDataModel::divideFactor(quint32 index) const
{
    return _graphData[index].divideFactor();
}

quint16 GraphDataModel::registerAddress(quint32 index) const
{
    return _graphData[index].registerAddress();
}

quint16 GraphDataModel::bitmask(quint32 index) const
{
    return _graphData[index].bitmask();
}

qint32 GraphDataModel::shift(quint32 index) const
{
    return _graphData[index].shift();
}

QSharedPointer<QCPGraphDataContainer> GraphDataModel::dataMap(quint32 index)
{
    return _graphData[index].dataMap();
}

void GraphDataModel::setVisible(quint32 index, bool bVisible)
{
    if (_graphData[index].isVisible() != bVisible)
    {
         _graphData[index].setVisible(bVisible);
         emit visibilityChanged(index);
    }
}

void GraphDataModel::setLabel(quint32 index, const QString &label)
{
    if (_graphData[index].label() != label)
    {
         _graphData[index].setLabel(label);
         emit labelChanged(index);
    }
}

void GraphDataModel::setColor(quint32 index, const QColor &color)
{
    if (_graphData[index].color() != color)
    {
         _graphData[index].setColor(color);
         emit colorChanged(index);
    }
}

void GraphDataModel::setActive(quint32 index, bool bActive)
{
    if (_graphData[index].isActive() != bActive)
    {
        _graphData[index].setActive(bActive);

        // Update activeGraphList
        updateActiveGraphList();

        // When deactivated, clear data
        if (!bActive)
        {
            _graphData[index].dataMap()->clear();
        }
        else
        {
            // when (re)-added, make sure graph is always visible
            _graphData[index].setVisible(true);
        }

        emit activeChanged(index);
    }
}

void GraphDataModel::setUnsigned(quint32 index, bool bUnsigned)
{
    if (_graphData[index].isUnsigned() != bUnsigned)
    {
         _graphData[index].setUnsigned(bUnsigned);
         emit unsignedChanged(index);
    }
}

void GraphDataModel::setMultiplyFactor(quint32 index, double multiplyFactor)
{
    if (_graphData[index].multiplyFactor() != multiplyFactor)
    {
         _graphData[index].setMultiplyFactor(multiplyFactor);
         emit multiplyFactorChanged(index);
    }
}

void GraphDataModel::setDivideFactor(quint32 index, double divideFactor)
{
    if (_graphData[index].divideFactor() != divideFactor)
    {
         _graphData[index].setDivideFactor(divideFactor);
         emit divideFactorChanged(index);
    }
}

void GraphDataModel::setRegisterAddress(quint32 index, const quint16 &registerAddress)
{
    if (_graphData[index].registerAddress() != registerAddress)
    {
         _graphData[index].setRegisterAddress(registerAddress);
         emit registerAddressChanged(index);
    }
}

void GraphDataModel::setBitmask(quint32 index, const quint16 &bitmask)
{
    if (_graphData[index].bitmask() != bitmask)
    {
         _graphData[index].setBitmask(bitmask);
         emit bitmaskChanged(index);
    }
}

void GraphDataModel::setShift(quint32 index, const qint32 &shift)
{
    if (_graphData[index].shift() != shift)
    {
         _graphData[index].setShift(shift);
         emit shiftChanged(index);
    }
}

void GraphDataModel::add(GraphData rowData)
{
    addToModel(&rowData);
}

void GraphDataModel::add(QList<GraphData> graphDataList)
{
    for (qint32 idx = 0; idx < graphDataList.size(); idx++)
    {
        add(graphDataList[idx]);
    }
}

void GraphDataModel::add()
{
    GraphData data;

    data.setRegisterAddress(nextFreeAddress());
    data.setLabel(QString("Register %1").arg(data.registerAddress()));

    add(data);
}

void GraphDataModel::add(QList<QString> labelList, QList<double> timeData, QList<QList<double> > data)
{
    foreach(QString label, labelList)
    {
        add();
        setLabel(_graphData.size() - 1, label);
    }

    emit graphsAddData(timeData, data);
}

void GraphDataModel::removeRegister(qint32 idx)
{   
    if (idx < _graphData.size())
    {
        removeFromModel(idx);
    }
}

void GraphDataModel::clear()
{
    const qint32 size = _graphData.size();

    for(qint32 idx = 0; idx < size; idx++)
    {
        removeRegister(0);
    }
}

// Get sorted list of active (unique) register addresses
void GraphDataModel::activeGraphAddresList(QList<quint16> * pRegisterList)
{
    // Clear list
    pRegisterList->clear();

    foreach(quint32 idx, _activeGraphList)
    {
        if (!pRegisterList->contains(_graphData[idx].registerAddress()))
        {
            pRegisterList->append(_graphData[idx].registerAddress());
        }
    }

    // sort qList
    qSort(*pRegisterList);
}

// Get list of active graph indexes
void GraphDataModel::activeGraphIndexList(QList<quint16> * pList)
{
    // Clear list
    pList->clear();

    foreach(quint32 idx, _activeGraphList)
    {
        pList->append(idx);
    }

    // sort qList
    qSort(*pList);
}

bool GraphDataModel::getDuplicate(quint16 * pRegister, quint16 * pBitmask)
{
    for (qint32 idx = 0; idx < (_graphData.size() - 1); idx++) // Don't need to check last entry
    {
        for (int checkIdx = (idx + 1); checkIdx < _graphData.size(); checkIdx++)
        {
            if (
                (_graphData[idx].registerAddress() == _graphData[checkIdx].registerAddress())
                && (_graphData[idx].bitmask() == _graphData[checkIdx].bitmask())
            )
            {
                *pRegister = _graphData[idx].registerAddress();
                *pBitmask = _graphData[idx].bitmask();
                return false;
            }
        }
    }

    return true;
}

bool GraphDataModel::isPresent(quint16 addr, quint16 bitmask)
{
    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (
            (_graphData[idx].registerAddress() == addr)
            && (_graphData[idx].bitmask() == bitmask)
        )
        {
            return false;
        }
    }

    return true;
}

qint32 GraphDataModel::convertToActiveGraphIndex(quint32 graphIdx)
{
    qint16 result = -1;
    for (qint32 activeIdx = 0; activeIdx < _activeGraphList.size(); activeIdx++)
    {
        if (_activeGraphList[activeIdx] == graphIdx)
        {
            result = activeIdx;
            break;
        }
    }
    return result;
}

qint32 GraphDataModel::convertToGraphIndex(quint32 activeIdx)
{
    return _activeGraphList[activeIdx];
}

quint16 GraphDataModel::nextFreeAddress()
{
    quint16 nextAddress = 40000;

    /* Find highest address of existing addresses */
    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].registerAddress() > nextAddress)
        {
            nextAddress = _graphData[idx].registerAddress();
        }
    }

    /* Set to next address */
    nextAddress++;

    return nextAddress;
}

void GraphDataModel::updateActiveGraphList(void)
{
    // Clear list
    _activeGraphList.clear();

    for (qint32 idx = 0; idx < _graphData.size(); idx++)
    {
        if (_graphData[idx].isActive())
        {
            _activeGraphList.append(idx);
        }
    }
}

void GraphDataModel::modelDataChanged(qint32 idx)
{
    // Notify view(s) of changes
    emit dataChanged(index(idx, 0), index(idx, columnCount() - 1));
}

void GraphDataModel::modelDataChanged(quint32 idx)
{
    modelDataChanged((qint32)idx);
}

void GraphDataModel::modelDataChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void GraphDataModel::addToModel(GraphData * pGraphData)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    /* Select color */
    if (!pGraphData->color().isValid())
    {
        quint32 colorIndex = _graphData.size() % Util::cColorlist.size();
        pGraphData->setColor(Util::cColorlist[colorIndex]);
    }

    _graphData.append(*pGraphData);

    updateActiveGraphList();

    /* Call function to trigger view update */
    endInsertRows();

    emit added(size() - 1);
}

void GraphDataModel::removeFromModel(qint32 row)
{
    beginRemoveRows(QModelIndex(), row, row);

    _graphData.removeAt(row);

    updateActiveGraphList();

    endRemoveRows();

    emit removed(row);
}
