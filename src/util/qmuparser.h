#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "muparserregister.h"
#include "result.h"

class QMuParser
{
public:
    QMuParser(QString strExpression);
    QMuParser(const QMuParser &source);
    ~QMuParser();

    void setExpression(QString expr);
    QString expression();

    static void setRegistersData(QList<Result>& regValues);

    bool evaluate();

    bool isSuccess();
    QString msg();
    double value();

private:

    void reset();

    static void registerValue(int index, int* value, bool* success);

    static QList<Result> _registerValues;

    mu::ParserRegister* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _value;
    QString _msg;

};

#endif // QMUPARSER_H
