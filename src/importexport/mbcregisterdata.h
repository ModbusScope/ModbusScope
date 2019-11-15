#ifndef MBCREGISTERDATA_H
#define MBCREGISTERDATA_H

#include <QtGlobal>
#include <QString>

class MbcRegisterData
{
public:
    explicit MbcRegisterData();
    MbcRegisterData(quint16 registerAddress, bool bUnsigned, QString name, qint32 tabIdx, bool bUint32, bool bReadable);
    ~MbcRegisterData();

    bool compare(MbcRegisterData* pMbcRegdata);

    quint16 registerAddress() const;
    void setRegisterAddress(const quint16 &registerAddress);

    bool isUnsigned() const;
    void setUnsigned(bool isUnsigned);

    QString name() const;
    void setName(const QString &name);

    qint32 tabIdx() const;
    void setTabIdx(const qint32 &tabIdx);

    bool is32Bit() const;
    void set32Bit(bool is32Bit);

    bool isReadable() const;
    void setReadable(bool isReadable);

private:

    quint16 _registerAddress;
    bool _bUnsigned;
    QString _name;
    qint32 _tabIdx;
    bool _bUint32;
    bool _bReadable;
};

#endif // MBCREGISTERDATA_H
