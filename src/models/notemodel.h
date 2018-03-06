#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <QObject>
#include <QModelIndex>
#include <QList>
#include <QAbstractTableModel>
#include "note.h"

class NoteModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit NoteModel(QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool removeRows (int row, int count, const QModelIndex & parent);

    Qt::ItemFlags flags(const QModelIndex& index) const;

    qint32 size() const;
    void add(Note &note);
    void remove(qint32 idx);

    double valueData(quint32 idx);
    double keyData(quint32 idx);
    QString textData(quint32 idx);
    bool draggable(quint32 idx);

    void setValueData(quint32 idx, double value);
    void setKeyData(quint32 idx, double key);
    void setText(quint32 idx, QString text);
    void setDraggable(quint32 idx, bool bState);

signals:
    void valueDataChanged(const quint32 idx);
    void keyDataChanged(const quint32 idx);
    void textChanged(const quint32 idx);
    void draggableChanged(const quint32 idx);

    void added(const quint32 idx); // When note is added
    void removed(const quint32 idx); // When note is removed

private slots:

    void modelDataChanged(quint32 idx);
    void modelDataChanged();

private:

    QList<Note> _noteList;

};

#endif // NOTEMODEL_H
