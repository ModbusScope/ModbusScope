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

    static void setRegistersData(QList<Result<double> > &regValues);

    bool evaluate();

    bool isSuccess() const;
    QString msg() const;
    qint32 errorPos() const;
    double value() const;

private:

    void reset();

    static void registerValue(int index, double *value, bool* success);

    static QList<Result<double> > _registerValues;

    mu::ParserRegister* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _value;
    QString _msg;
    qint32 _errorPos;

};

#endif // QMUPARSER_H
