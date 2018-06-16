#include "errorlogmodel.h"
#include "QModelIndex"
#include "QAbstractItemModel"

/*!
 * \brief Constructor for ErrorLogModel
 * \param parent    parent object
 */
ErrorLogModel::ErrorLogModel(QObject *parent) : QAbstractListModel(parent)
{

}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
int ErrorLogModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

/*!
 * \brief Return numbers of columns in model
 * \return Numbers of columns in model
 */
int ErrorLogModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

/*!
 * \brief Get data from model
 * \param index modelindex referring to requested data
 * \param role Requested data role
 * \return Requested data from model, Emptye QVariant() on invalid argument
 */
QVariant ErrorLogModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (role == Qt::DisplayRole))
    {
        return _logList[index.row()].toString();
    }

    return QVariant();
}

/*!
 * \brief Get header data from model
 * \param section Requested column header data
 * \param orientation Unused
 * \param role Requested header role
 * \return Requested header data from model, Emptye QVariant() on invalid argument
 */
QVariant ErrorLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (
            (section == 0)
            && (role == Qt::DisplayRole)
        )
    {
        return QString("Messages");
    }

    return QVariant();
}

/*!
 * \brief Get flags for specific index
 * \param index modelindex referring to requested data
 * \return Flags of index
 */
Qt::ItemFlags ErrorLogModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
qint32 ErrorLogModel::size() const
{
    return _logList.size();
}

/*!
 * \brief Clear the model data
 */
void ErrorLogModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, size());

    _logList.clear();

    endRemoveRows();
}

/*!
 * \brief Add item to model
 * \param log
 */
void ErrorLogModel::addItem(ErrorLog log)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    _logList.append(log);

    /* Call functions to trigger view update */
    endInsertRows();

    QModelIndex nIndex = index(size() - 1, 0);

    emit dataChanged(nIndex, nIndex);
}
