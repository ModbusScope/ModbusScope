#ifndef MBCREGISTERDATA_H
#define MBCREGISTERDATA_H

#include "util/modbusdatatype.h"

#include <QString>
#include <QtGlobal>

class MbcRegisterData
{
public:
    explicit MbcRegisterData();
    MbcRegisterData(quint32 registerAddress, ModbusDataType::Type type, QString name, qint32 tabIdx, bool bReadable, quint8 decimals);
    ~MbcRegisterData();

    bool compare(MbcRegisterData* pMbcRegdata);

    quint32 registerAddress() const;
    void setRegisterAddress(const quint32 &registerAddress);

    void setType(ModbusDataType::Type type);

    ModbusDataType::Type type() const;

    QString name() const;
    void setName(const QString &name);

    qint32 tabIdx() const;
    void setTabIdx(const qint32 &tabIdx);

    bool isReadable() const;
    void setReadable(bool isReadable);

    quint8 decimals() const;
    void setDecimals(const quint8 &decimals);

    QString toExpression();

private:

    quint32 _registerAddress;
    QString _name;
    qint32 _tabIdx;
    ModbusDataType::Type _type;
    bool _bReadable;
    quint8 _decimals;
};

#endif // MBCREGISTERDATA_H
