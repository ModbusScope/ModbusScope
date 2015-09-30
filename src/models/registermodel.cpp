#include "registermodel.h"
#include "util.h"
#include "QDebug"
#include <QMessageBox>

RegisterModel::RegisterModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    _dataList.clear();
}

int RegisterModel::rowCount(const QModelIndex & /*parent*/) const
{
    return _dataList.size();
}

int RegisterModel::columnCount(const QModelIndex & /*parent*/) const
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

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{

    switch (index.column())
    {
    case 0:
        if (role == Qt::BackgroundColorRole)
        {
            return _dataList[index.row()].color();
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (_dataList[index.row()].isActive())
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
            if (_dataList[index.row()].isUnsigned())
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
            return _dataList[index.row()].registerAddress();
        }
        break;
    case 4:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _dataList[index.row()].text();
        }
        break;
    case 5:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            // Show hex value
            return QString("0x%1").arg(_dataList[index.row()].bitmask(), 0, 16);
        }
        break;
    case 6:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _dataList[index.row()].shift();
        }
        break;
    case 7:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(_dataList[index.row()].multiplyFactor());
        }
        break;
    case 8:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(_dataList[index.row()].divideFactor());
        }
        break;
    default:
        return QVariant();
        break;

    }

    return QVariant();
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
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


bool RegisterModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case 0:
        if (role == Qt::EditRole)
        {
            QColor color = value.value<QColor>();
            _dataList[index.row()].setColor(color);
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _dataList[index.row()].setActive(true);
            }
            else
            {
                _dataList[index.row()].setActive(false);
            }
        }
        break;
    case 2:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _dataList[index.row()].setUnsigned(true);
            }
            else
            {
                _dataList[index.row()].setUnsigned(false);
            }
        }
        break;
    case 3:
        if (role == Qt::EditRole)
        {
            const quint16 newAddr = value.toInt();
            _dataList[index.row()].setRegisterAddress(newAddr);
        }
        break;
    case 4:
        if (role == Qt::EditRole)
        {
            _dataList[index.row()].setText(value.toString());
        }
        break;
    case 5:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint16 newBitMask = value.toString().toUInt(&bSuccess, 0);

            if (bSuccess)
            {
                _dataList[index.row()].setBitmask(newBitMask);
            }
            else
            {
                bRet = false;
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("ModbusScope data error"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Bitmask is not a valid integer."));
                msgBox.exec();
                break;
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
                _dataList[index.row()].setShift(newShift);
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
                _dataList[index.row()].setMultiplyFactor(parseResult);
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
                _dataList[index.row()].setDivideFactor(parseResult);
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

Qt::ItemFlags RegisterModel::flags(const QModelIndex & index) const
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


bool RegisterModel::removeRows (int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (qint32 i = 0; i < count; i++)
    {
        Q_UNUSED(i);
        if (row < _dataList.size())
        {
            _dataList.removeAt(row);
        }
    }
    endRemoveRows();

    // Notify view(s) of change
    emit dataChanged(parent, parent);

    return true;
}

bool RegisterModel::insertRows(RegisterData data, int row, int count, const QModelIndex &parent)
{

    if (
        (count != 1)
        || (row != _dataList.size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, _dataList.size(), _dataList.size());

    /* Select color when adding through register dialog */
    if (!data.color().isValid())
    {
        quint32 colorIndex = _dataList.size() % Util::cColorlist.size();
        data.setColor(Util::cColorlist[colorIndex]);
    }

    _dataList.append(data);

    endInsertRows();

    // Notify view(s) of change
    emit dataChanged(parent, parent);

    return true;
}


bool RegisterModel::insertRows (int row, int count, const QModelIndex &parent)
{
    RegisterData data;
    data.setActive(true);
    data.setUnsigned(false);
    data.setRegisterAddress(nextFreeAddress());
    data.setBitmask(0xFFFF);
    data.setText(QString("Register %1 (bitmask: 0x%2)").arg(data.registerAddress()).arg(data.bitmask(), 0, 16));
    data.setDivideFactor(1);
    data.setMultiplyFactor(1);
    data.setShift(0);
    data.setColor("-1"); // Invalid color

    return insertRows(data, row, count, parent);;
}

uint RegisterModel::checkedRegisterCount()
{
    uint count = 0;

    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].isActive())
        {
            count++;
        }
    }

    return count;
}

RegisterData RegisterModel::registerAtIndex(qint32 index)
{
    RegisterData retRegister;

    _dataList[index].CopyTo(&retRegister);

    return retRegister;
}

void RegisterModel::registerList(QList<quint16> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < _dataList.size(); i++)
    {
        pRegisterList->append(_dataList[i].registerAddress());
    }
}

/*
 *  Get sorted list of checked register addresses
 */
void RegisterModel::checkedRegisterList(QList<RegisterData> * pRegisterList)
{
    pRegisterList->clear();
    for (int i = 0; i < _dataList.size(); i++)
    {
        if (_dataList[i].isActive())
        {
            pRegisterList->append(_dataList[i]);
        }
    }

    // Sort by register addresscolor
    std::sort(pRegisterList->begin(), pRegisterList->end(), &RegisterData::sortRegisterDataList);
}

void RegisterModel::clear(const QModelIndex &parent)
{
    if (_dataList.size() != 0)
    {
        removeRows(0, _dataList.size(), QModelIndex());
    }

    // Notify view(s) of change
    emit dataChanged(parent, parent);
}

void RegisterModel::appendRow(RegisterData rowData, const QModelIndex &parent)
{
    insertRows(rowData, _dataList.size(), 1, parent);
}

bool RegisterModel::areExclusive(quint16 * pRegister, quint16 * pBitmask)
{
    for (qint32 idx = 0; idx < (_dataList.size() - 1); idx++) // Don't need to check last entry
    {
        for (int checkIdx = (idx + 1); checkIdx < _dataList.size(); checkIdx++)
        {
            if (
                (_dataList[idx].registerAddress() == _dataList[checkIdx].registerAddress())
                && (_dataList[idx].bitmask() == _dataList[checkIdx].bitmask())
            )
            {
                *pRegister = _dataList[idx].registerAddress();
                *pBitmask = _dataList[idx].bitmask();
                return false;
            }
        }
    }

    return true;
}

quint16 RegisterModel::nextFreeAddress()
{
    QList<quint16> regList;
    quint16 nextAddress;

    // get register list
    this->registerList(&regList);

    // sort qList
    qSort(regList);

    if (regList.size() > 0)
    {
        nextAddress = regList.last() + 1;
    }
    else
    {
        nextAddress = 40001;
    }

    return nextAddress;
}