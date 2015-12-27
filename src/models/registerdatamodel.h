#ifndef REGISTERDATAMODEL_H
#define REGISTERDATAMODEL_H

#include <QObject>
#include "registerdata.h"

class RegisterDataModel : public QObject
{
    Q_OBJECT
public:
    explicit RegisterDataModel(QObject *parent = 0);

    qint32 size();

    RegisterData * registerData(qint32 idx);

    void addRegister(RegisterData rowData);
    void addRegister();
    void removeRegister(qint32 idx);
    void clear();

    uint checkedRegisterCount();
    void registerList(QList<quint16> * pRegisterList);
    void checkedRegisterList(QList<RegisterData> * pRegisterList);

    bool areExclusive(quint16 * pRegister, quint16 *pBitmask);

signals:

    void removed(qint32 idx);
    void added(qint32 idx);
    void cleared();

public slots:

private:
    quint16 nextFreeAddress();

    QList<RegisterData> _dataList;
};

#endif // REGISTERDATAMODEL_H
