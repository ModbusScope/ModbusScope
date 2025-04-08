#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "communication/modbusregister.h"
#include <QRegularExpression>
#include <QStringList>

class ExpressionParser : public QObject
{
    Q_OBJECT
public:
    explicit ExpressionParser(QStringList& expressions);

    void modbusRegisters(QList<ModbusRegister>& registerList);
    void processedExpressions(QStringList& expressionList);

private:

    void parseExpressions(QStringList& expressions);

    bool parseAddress(QString strAddr, ModbusRegister& modbusReg);
    bool parseConnectionId(QString strConnectionId, ModbusRegister& modbusReg);
    bool parseType(QString strType, ModbusRegister& modbusReg);

    QString processExpression(QString const & expr);
    bool processRegisterExpression(QString regExpr, ModbusRegister &modbusReg);
    QString constructInternalRegisterFunction(ModbusRegister const & modbusReg, int size);

    QStringList _processedExpressions;
    QList<ModbusRegister> _modbusRegisters;

    QRegularExpression _findRegRegex;
    QRegularExpression _regParseRegex;

    static const QString _cRegisterFunctionTemplate;

};

#endif // EXPRESSIONPARSER_H
