#ifndef GENERALERROR_H
#define GENERALERROR_H

#include <QString>

class GeneralError
{

public:
    explicit GeneralError() : GeneralError(true) {};
    explicit GeneralError(bool result)
    {
        _result = result;
        _msg = QString();
    }

    bool result() const
    {
        return _result;
    };
    QString msg() const
    {
        return _msg;
    };

    void setResult(bool result)
    {
        _result = result;
    };

    void reportError(QString msg)
    {
        _msg = msg;
        _result = false;
    };

private:
    bool _result;
    QString _msg;
};

#endif // GENERALERROR_H
