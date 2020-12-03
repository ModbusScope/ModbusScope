#include "diagnosticmodel.h"
#include "QModelIndex"
#include "QAbstractItemModel"

/*!
 * \brief Constructor for DiagnosticModel
 * \param parent    parent object
 */
DiagnosticModel::DiagnosticModel(QObject *parent) : QAbstractListModel(parent)
{

}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
int DiagnosticModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

/*!
 * \brief Return numbers of columns in model
 * \return Numbers of columns in model
 */
int DiagnosticModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

/*!
 * \brief Get data from model
 * \param index modelindex referring to requested data
 * \param role Requested data role
 * \return Requested data from model, Empty QVariant() on invalid argument
 */
QVariant DiagnosticModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && (role == Qt::DisplayRole))
    {
        return _logList[index.row()].toString();
    }

    return QVariant();
}

/*!
 * \brief Get data severity from model
 * \param index row referring to requested data
 * \return Requested data from model, -1 on invalid argument
 */
Diagnostic::LogSeverity DiagnosticModel::dataSeverity(quint32 index) const
{
    if (index < static_cast<quint32>(size()))
    {
        return _logList[static_cast<qint32>(index)].severity();
    }

    return static_cast<Diagnostic::LogSeverity>(-1);
}

/*!
 * \brief Get header data from model
 * \param section Requested column header data
 * \param orientation Unused
 * \param role Requested header role
 * \return Requested header data from model, Emptye QVariant() on invalid argument
 */
QVariant DiagnosticModel::headerData(int section, Qt::Orientation orientation, int role) const
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
Qt::ItemFlags DiagnosticModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
 * \brief Return numbers of rows in model
 * \return Numbers of rows in model
 */
qint32 DiagnosticModel::size() const
{
    return _logList.size();
}

/*!
 * \brief Clear the model data
 */
void DiagnosticModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, size() - 1);

    _logList.clear();

    endRemoveRows();
}

QString DiagnosticModel::toString(quint32 idx) const
{
    return _logList[idx].toString();
}

QString DiagnosticModel::toExportString(quint32 idx) const
{
    return _logList[idx].toExportString();
}

/*!
 * \brief Add item to model
 * \param log
 */
void DiagnosticModel::addLog(QString category, Diagnostic::LogSeverity severity, qint32 timeOffset, QString message)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    Diagnostic log(category, severity, timeOffset, message);

    _logList.append(log);

    /* Call functions to trigger view update */
    endInsertRows();

    QModelIndex nIndex = index(size() - 1, 0);

    emit dataChanged(nIndex, nIndex);
}
