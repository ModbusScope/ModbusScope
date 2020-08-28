#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "muParser.h"

class QMuParser
{
public:
    QMuParser(QString strExpression);
    ~QMuParser();

    bool evaluate();
    bool evaluate(double regValue);

    bool isSuccess();
    QString msg();
    double result();

private:

    void reset();

    mu::ParserBase* _pExprParser;

    double _registerValue;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _result;
    QString _msg;

};

#endif // QMUPARSER_H
