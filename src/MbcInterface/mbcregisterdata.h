#ifndef MBCREGISTERDATA_H
#define MBCREGISTERDATA_H

#include "MbcInterface/mbcdatatype.h"

#include <QString>
#include <QtGlobal>

class MbcRegisterData
{
public:
    explicit MbcRegisterData();
    MbcRegisterData(
      quint32 registerAddress, MbcDataType::Type type, QString name, qint32 tabIdx, bool bReadable, quint8 decimals);
    ~MbcRegisterData();

    bool compare(MbcRegisterData* pMbcRegdata);

    quint32 registerAddress() const;
    void setRegisterAddress(const quint32& registerAddress);

    void setType(MbcDataType::Type type);

    MbcDataType::Type type() const;

    QString name() const;
    void setName(const QString& name);

    qint32 tabIdx() const;
    void setTabIdx(const qint32& tabIdx);

    bool isReadable() const;
    void setReadable(bool isReadable);

    quint8 decimals() const;
    void setDecimals(const quint8& decimals);

    QString toExpression() const;

private:
    quint32 _registerAddress;
    QString _name;
    qint32 _tabIdx;
    MbcDataType::Type _type;
    bool _bReadable;
    quint8 _decimals;
};

#endif // MBCREGISTERDATA_H
