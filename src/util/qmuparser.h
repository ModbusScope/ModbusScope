#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "muParser.h"
#include "modbusresult.h"

class QMuParser
{
public:
    QMuParser(QString strExpression);
    ~QMuParser();

    void setExpression(QString expr);
    QString expression();

    static void setRegistersData(QList<ModbusResult>& regValues);

    bool evaluate();

    bool isSuccess();
    QString msg();
    double result();

private:

    void reset();

    static void registerValue(int index, int* value, bool* success);

    static QList<ModbusResult> _registerValues;

    mu::ParserBase* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _result;
    QString _msg;

};

#endif // QMUPARSER_H
