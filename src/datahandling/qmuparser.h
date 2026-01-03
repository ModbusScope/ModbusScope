#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "datahandling/muparserregister.h"
#include "util/result.h"

class QMuParser
{

public:

    enum class ErrorType
    {
        NONE = 0,
        SYNTAX,
        OTHER,
    };

    QMuParser(QString strExpression);
    QMuParser(const QMuParser &source);
    ~QMuParser();

    void setExpression(QString expr);
    QString expression();

    static void setRegistersData(ResultDoubleList &regValues);

    bool evaluate();

    QString originalExpression() const;
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
    QString _originalExpression;
    double _value;
    QString _msg;
    qint32 _errorPos;
    ErrorType _errorType;

};

#endif // QMUPARSER_H
