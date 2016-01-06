#include "registerdialogmodel.h"
#include "util.h"
#include "QDebug"
#include <QMessageBox>

RegisterDialogModel::RegisterDialogModel(GraphDataModel * pGraphDataModel, QObject *parent) :
    QAbstractTableModel(parent)
{
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphDataModel, SIGNAL(removed(qint32)), this, SLOT(modelDataChanged(qint32)));
    connect(_pGraphDataModel, SIGNAL(added(qint32)), this, SLOT(modelDataChanged(qint32)));

    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(unsignedChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(multiplyFactorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(divideFactorChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(registerAddressChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(bitmaskChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(_pGraphDataModel, SIGNAL(shiftChanged(quint32)), this, SLOT(modelDataChanged(quint32)));

    connect(_pGraphDataModel, SIGNAL(cleared()), this, SLOT(modelDataChanged()));
}

int RegisterDialogModel::rowCount(const QModelIndex & /*parent*/) const
{
    return _pGraphDataModel->size();
}

int RegisterDialogModel::columnCount(const QModelIndex & /*parent*/) const
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

QVariant RegisterDialogModel::data(const QModelIndex &index, int role) const
{

    switch (index.column())
    {
    case 0:
        if (role == Qt::BackgroundColorRole)
        {
            return _pGraphDataModel->color(index.row());
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (_pGraphDataModel->isActive(index.row()))
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
            if (_pGraphDataModel->isUnsigned(index.row()))
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
            return _pGraphDataModel->registerAddress(index.row());
        }
        break;
    case 4:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _pGraphDataModel->label(index.row());
        }
        break;
    case 5:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            // Show hex value
            return QString("0x%1").arg(_pGraphDataModel->bitmask(index.row()), 0, 16);
        }
        break;
    case 6:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _pGraphDataModel->shift(index.row());
        }
        break;
    case 7:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(_pGraphDataModel->multiplyFactor(index.row()));
        }
        break;
    case 8:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return Util::formatDoubleForExport(_pGraphDataModel->divideFactor(index.row()));
        }
        break;
    default:
        return QVariant();
        break;

    }

    return QVariant();
}

QVariant RegisterDialogModel::headerData(int section, Qt::Orientation orientation, int role) const
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


bool RegisterDialogModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case 0:
        if (role == Qt::EditRole)
        {
            QColor color = value.value<QColor>();
            _pGraphDataModel->setColor(index.row(), color);
        }
        break;
    case 1:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _pGraphDataModel->setActive(index.row(), true);
            }
            else
            {
                _pGraphDataModel->setActive(index.row(), false);
            }
        }
        break;
    case 2:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                _pGraphDataModel->setUnsigned(index.row(), true);
            }
            else
            {
                _pGraphDataModel->setUnsigned(index.row(), false);
            }
        }
        break;
    case 3:
        if (role == Qt::EditRole)
        {
            const quint16 newAddr = value.toInt();
            _pGraphDataModel->setRegisterAddress(index.row(), newAddr);
        }
        break;
    case 4:
        if (role == Qt::EditRole)
        {
            _pGraphDataModel->setLabel(index.row(), value.toString());
        }
        break;
    case 5:
        if (role == Qt::EditRole)
        {
            bool bSuccess = false;
            const quint16 newBitMask = value.toString().toUInt(&bSuccess, 0);

            if (bSuccess)
            {
                _pGraphDataModel->setBitmask(index.row(), newBitMask);
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
                _pGraphDataModel->setShift(index.row(), newShift);
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
                _pGraphDataModel->setMultiplyFactor(index.row(), parseResult);
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
                _pGraphDataModel->setDivideFactor(index.row(), parseResult);
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

Qt::ItemFlags RegisterDialogModel::flags(const QModelIndex & index) const
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


bool RegisterDialogModel::removeRows (int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    for (qint32 i = 0; i < count; i++)
    {
        Q_UNUSED(i);
        _pGraphDataModel->removeRegister((qint32)row);
    }

    endRemoveRows();

    return true;
}

bool RegisterDialogModel::insertRows (int row, int count, const QModelIndex &parent)
{
    if (
        (count != 1)
        || (row != _pGraphDataModel->size())
        )
    {
        qDebug() << "RegisterModel: Not supported";
    }

    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, _pGraphDataModel->size(), _pGraphDataModel->size());

    _pGraphDataModel->add();

    endInsertRows();

    return true;
}

void RegisterDialogModel::modelDataChanged(qint32 idx)
{
    // Notify view(s) of changes
    emit dataChanged(createIndex(idx, 0), createIndex(idx, columnCount()));
}

void RegisterDialogModel::modelDataChanged(quint32 idx)
{
    // Notify view(s) of changes
    emit dataChanged(createIndex((qint32)idx, 0), createIndex((qint32)idx, columnCount()));
}

void RegisterDialogModel::modelDataChanged()
{
    emit dataChanged(createIndex(0, 0), createIndex(rowCount(), columnCount()));
}
