#include "notemodel.h"

#include "util.h"

NoteModel::NoteModel(QObject *parent) : QAbstractTableModel(parent)
{
    _noteList.clear();

    connect(this, SIGNAL(valueDataChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(keyDataChanged(quint32)), this, SLOT(modelDataChanged(quint32)));
    connect(this, SIGNAL(textChanged(quint32)), this, SLOT(modelDataChanged(quint32)));

    connect(this, SIGNAL(added(quint32)), this, SLOT(modelDataChanged()));
    connect(this, SIGNAL(removed(quint32)), this, SLOT(modelDataChanged()));
}

QVariant NoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return QString("Draggable");
            case 1:
                return QString("Key");
            case 2:
                return QString("Value");
            case 3:
                return QString("Text");
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

int NoteModel::rowCount(const QModelIndex & /*parent*/) const
{
    return size();
}

int NoteModel::columnCount(const QModelIndex & /*parent*/) const
{
    /*
    * draggable
    * keyData
    * valueData
    * text
    * */
    return 4; // Number of visible members of struct
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    switch (index.column())
    {
    case 0:
        if (role == Qt::CheckStateRole)
        {
            if (_noteList[index.row()].draggable())
            {
                return Qt::Checked;
            }
            else
            {
                return Qt::Unchecked;
            }
        }
        break;
    case 1:
        if ((role == Qt::DisplayRole))
        {
            return Util::formatTime(_noteList[index.row()].keyData(), false);
        }
        break;
    case 2:
        if ((role == Qt::DisplayRole))
        {
            return Util::formatDoubleForExport(_noteList[index.row()].valueData());
        }
        break;
    case 3:
        if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        {
            return _noteList[index.row()].text();
        }
        break;

    default:
        return QVariant();
        break;

    }

    return QVariant();
}

bool NoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bRet = true;

    switch (index.column())
    {
    case 0:
        if (role == Qt::CheckStateRole)
        {
            if (value == Qt::Checked)
            {
                setDraggable(index.row(), true);
            }
            else
            {
                setDraggable(index.row(), false);
            }
        }
        break;

    case 3:
        if (role == Qt::EditRole)
        {
            setText(index.row(), value.toString());
        }
        break;
    default:
        bRet = false;
        break;
    }

    // Notify view(s) of change
    emit dataChanged(index, index);

    return bRet;
}

bool NoteModel::removeRows (int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(count);

    remove(row);

    return true;
}

Qt::ItemFlags NoteModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        return Qt::ItemIsSelectable |  Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    }
    else if (index.column() == 3)
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

qint32 NoteModel::size() const
{
    return _noteList.size();
}

void NoteModel::add(Note &note)
{
    /* Call function to prepare view */
    beginInsertRows(QModelIndex(), size(), size());

    _noteList.append(note);
    setNotesDataUpdated(true);

    /* Call function to trigger view update */
    endInsertRows();

    emit added(size() - 1);
}

void NoteModel::remove(qint32 idx)
{
    beginRemoveRows(QModelIndex(), idx, idx);

    _noteList.removeAt(idx);
    setNotesDataUpdated(true);

    endRemoveRows();

    emit removed(idx);
}

void NoteModel::clear()
{
    qint32 count = size();
    for(qint32 idx = 0; idx < count; idx++)
    {
        remove(0);
    }
}

double NoteModel::valueData(quint32 idx)
{
    return _noteList[idx].valueData();
}

double NoteModel::keyData(quint32 idx)
{
    return _noteList[idx].keyData();
}

QString NoteModel::textData(quint32 idx)
{
    return _noteList[idx].text();
}

bool NoteModel::draggable(quint32 idx)
{
    return _noteList[idx].draggable();
}

bool NoteModel::isNotesDataUpdated()
{
    return _bNotesDataUpdated;
}

void NoteModel::setValueData(quint32 idx, double value)
{
    if (_noteList[idx].valueData() != value)
    {
         _noteList[idx].setValueData(value);
         setNotesDataUpdated(true);
         emit valueDataChanged(idx);
    }
}

void NoteModel::setKeyData(quint32 idx, double key)
{
    if (_noteList[idx].keyData() != key)
    {
         _noteList[idx].setKeyData(key);
         setNotesDataUpdated(true);
         emit keyDataChanged(idx);
    }
}

void NoteModel::setText(quint32 idx, QString text)
{
    if (_noteList[idx].text() != text)
    {
         _noteList[idx].setText(text);
         setNotesDataUpdated(true);
         emit textChanged(idx);
    }
}

void NoteModel::setDraggable(quint32 idx, bool bState)
{
    if (_noteList[idx].draggable() != bState)
    {
         _noteList[idx].setDraggable(bState);
         emit draggableChanged(idx);
    }
}

void NoteModel::setNotesDataUpdated(bool bUpdated)
{
    if (_bNotesDataUpdated != bUpdated)
    {
         _bNotesDataUpdated = bUpdated;
         emit notesDataUpdatedChanged();
    }
}

void NoteModel::modelDataChanged(quint32 idx)
{
    // Notify view(s) of changes
    emit dataChanged(index(idx, 0), index(idx, columnCount() - 1));
}

void NoteModel::modelDataChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
