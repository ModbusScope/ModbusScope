#ifndef REGISTERDATA_H
#define REGISTERDATA_H

#include <QColor>

class RegisterData
{

public:
    RegisterData();

    void CopyTo(RegisterData * pData);

    bool isActive() const;
    void setActive(bool value);

    bool isUnsigned() const;
    void setUnsigned(bool value);

    double scaleFactor() const;
    void setScaleFactor(double value);

    quint16 registerAddress() const;
    void setRegisterAddress(const quint16 &value);

    quint16 bitmask() const;
    void setBitmask(const quint16 &value);

    QString text() const;
    void setText(const QString &value);

    QColor color() const;
    void setColor(const QColor &value);

    qint32 shift() const;
    void setShift(const qint32 &value);

    static bool sortRegisterDataList(const RegisterData& s1, const RegisterData& s2);

private:

    bool _bActive;
    bool _bUnsigned;
    double _scaleFactor;
    quint16 _reg;
    quint16 _bitmask;
    QString _text;
    QColor _color;
    qint32 _shift;
};

#endif // REGISTERDATA_H
