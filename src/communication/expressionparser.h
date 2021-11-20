#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <QRegularExpression>
#include <QStringList>
#include "modbusregister.h"

class ExpressionParser : public QObject
{
    Q_OBJECT
public:
    ExpressionParser(QStringList& expressions);

    void modbusRegisters(QList<ModbusRegister>& registerList);
    void processedExpressions(QStringList& expressionList);

private:

    void parseExpressions(QStringList& expressions);

    QString processExpression(QString& expr);
    bool processRegisterExpression(QString regExpr, ModbusRegister &modbusReg);

    QStringList _processedExpressions;
    QList<ModbusRegister> _modbusRegisters;

    QRegularExpression _findRegRegex;
    QRegularExpression _regParseRegex;

    static const QString _cRegisterPattern;
    static const QString _cParseRegPattern;

};

#endif // EXPRESSIONPARSER_H
