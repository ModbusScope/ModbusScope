#ifndef REGISTERMODEL_H
#define REGISTERMODEL_H

#include <QAbstractTableModel>

class RegisterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RegisterModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);

    uint checkedRegisterCount();
    void getCheckedRegisterList(QList<quint16> * pRegisterList);
    void getCheckedRegisterTextList(QList<QString> * pRegisterTextList);

signals:

public slots:

private:

    typedef struct
    {
        bool bActive;
        quint16 reg;
        QString text;
    } RegisterData;

    QList<RegisterData> dataList;

};

#endif // REGISTERMODEL_H
