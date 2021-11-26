#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "muParser.h"

class QMuParser
{
public:
    QMuParser(QString strExpression);
    ~QMuParser();

    void setExpression(QString expr);
    QString expression();

    void setRegisterValues(QList<double>& regValues);

    bool evaluate();

    bool isSuccess();
    QString msg();
    double result();

private:

    void reset();

    static int registerValue(int index);

    static QList<double> _registerValues;

    mu::ParserBase* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _result;
    QString _msg;

};

#endif // QMUPARSER_H
