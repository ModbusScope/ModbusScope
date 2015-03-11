#ifndef REGISTERDATA_H
#define REGISTERDATA_H

#include <QColor>

class RegisterData
{

public:
    RegisterData();

    void CopyTo(RegisterData * pData);

    bool getActive() const;
    void setActive(bool value);

    bool getUnsigned() const;
    void setUnsigned(bool value);

    double getScaleFactor() const;
    void setScaleFactor(double value);

    quint16 getRegisterAddress() const;
    void setRegisterAddress(const quint16 &value);

    QString getText() const;
    void setText(const QString &value);

    static bool sortRegisterDataList(const RegisterData& s1, const RegisterData& s2);

private:

    bool bActive;
    bool bUnsigned;
    double scaleFactor;
    quint16 reg;
    QString text;
};

#endif // REGISTERDATA_H
