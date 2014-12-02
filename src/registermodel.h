#ifndef REGISTERMODEL_H
#define REGISTERMODEL_H

#include <QAbstractTableModel>

class RegisterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RegisterModel(QObject *parent = 0);

    typedef struct
    {
        bool bActive;
        quint16 reg;
        QString text;
    } RegisterData;

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool removeRows (int row, int count, const QModelIndex &parent);
    bool insertRows (int row, int count, const QModelIndex &parent);

    uint checkedRegisterCount();
    void getRegisterList(QList<quint16> * pRegisterList);
    void getCheckedRegisterList(QList<quint16> * pRegisterList);
    void getCheckedRegisterTextList(QList<QString> * pRegisterTextList);

    void clear();
    void appendRow(RegisterData rowData);

signals:

public slots:

private:

    bool IsAlreadyPresent(quint16 newReg);
    quint16 getNextFreeAddress();

    QList<RegisterData> dataList;

};

#endif // REGISTERMODEL_H
