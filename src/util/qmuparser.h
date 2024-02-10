#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "muparserregister.h"
#include "result.h"

class QMuParser
{

public:

    enum class ErrorType
    {
        NO_ERROR = 0,
        SYNTAX_ERROR,
        OTHER_ERROR,
    };

    QMuParser(QString strExpression);
    QMuParser(const QMuParser &source);
    ~QMuParser();

    void setExpression(QString expr);
    QString expression();

    static void setRegistersData(ResultDoubleList &regValues);

    bool evaluate();

    bool isSuccess() const;
    QString msg() const;
    qint32 errorPos() const;
    ErrorType errorType() const;
    double value() const;

private:

    void reset();

    static void registerValue(int index, double *value, bool* success);

    static ResultDoubleList _registerValues;

    mu::ParserRegister* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _value;
    QString _msg;
    qint32 _errorPos;
    ErrorType _errorType;

};

#endif // QMUPARSER_H
