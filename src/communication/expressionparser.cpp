#include "expressionparser.h"

#include "scopelogging.h"

const QString ExpressionParser::_cRegisterPattern = "\\${(\\d?.*?)}";
const QString ExpressionParser::_cParseRegPattern = "(\\d+)(?:@(\\d+))?(?:\\:(\\w+))?";

const QString ExpressionParser::_cRegisterFunctionTemplate = "regval(%1)";

ExpressionParser::ExpressionParser(QStringList& expressions)
{
    _findRegRegex.setPattern(_cRegisterPattern);
    _findRegRegex.optimize();

    _regParseRegex.setPattern(_cParseRegPattern);
    _regParseRegex.optimize();

    parseExpressions(expressions);
}

void ExpressionParser::modbusRegisters(QList<ModbusRegister>& registerList)
{
    registerList = _modbusRegisters;
}

void ExpressionParser::processedExpressions(QStringList& expressionList)
{
    expressionList = _processedExpressions;
}

void ExpressionParser::parseExpressions(QStringList& expressions)
{
    _processedExpressions.clear();
    _modbusRegisters.clear();

    for(QString expression: qAsConst(expressions))
    {
        _processedExpressions.append(processExpression(expression));
    }
}

QString ExpressionParser::processExpression(QString& graphExpr)
{
    QString resultExpr = graphExpr;
    resultExpr = resultExpr.remove(' ');
    QRegularExpressionMatchIterator i = _findRegRegex.globalMatch(resultExpr);

    if (!i.hasNext() && resultExpr.contains("$"))
    {
        auto msg = QString("Expression evaluation parsing failed (\"%1\")").arg(resultExpr);
        qCWarning(scopeComm) << msg;
    }

    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch())
        {
            QString regDef = match.captured(0);

            ModbusRegister modbusReg;
            if (processRegisterExpression(regDef, modbusReg))
            {
                quint8 idx;
                if (_modbusRegisters.contains(modbusReg))
                {
                    idx = _modbusRegisters.indexOf(modbusReg);
                }
                else
                {
                    _modbusRegisters.append(modbusReg);
                    idx = _modbusRegisters.size() - 1;
                }

                QString regFunc = QString(_cRegisterFunctionTemplate).arg(idx);
                resultExpr.replace(regDef, regFunc);
            }
            else
            {
                resultExpr.replace(regDef, QString());
            }
        }
    }

    return resultExpr;
}

bool ExpressionParser::processRegisterExpression(QString regExpr, ModbusRegister& modbusReg)
{
    bool bRet = false;

    QRegularExpressionMatch match = _regParseRegex.match(regExpr);

    QString strAddress;
    QString strConnectionId;
    QString strType;

    if (match.hasMatch())
    {
        strAddress = match.captured(1);
        strConnectionId = match.captured(2);
        strType = match.captured(3);

        bRet = true;
        bRet = bRet && parseAddress(strAddress, modbusReg);
        bRet = bRet && parseConnectionId(strConnectionId, modbusReg);
        bRet = bRet && parseType(strType, modbusReg);
    }
    else
    {
        auto msg = QString("Part of expression evaluation parsing failed (\"%1\")").arg(regExpr);
        qCWarning(scopeComm) << msg;
        bRet = false;
    }

    return bRet;
}

bool ExpressionParser::parseAddress(QString strAddr, ModbusRegister& modbusReg)
{
    bool bRet = false;

    if (strAddr.isEmpty())
    {
        /* Required field */
        bRet = false;
        qCWarning(scopeComm) << QString("No address specified");
    }
    else
    {
        quint16 addr = static_cast<quint16>(strAddr.toUInt(&bRet));
        if (bRet)
        {
            modbusReg.setAddress(addr);
        }
        else
        {
            qCWarning(scopeComm) << QString("Parsing address \"%1\" failed").arg(strAddr);
        }
    }

    return bRet;
}

bool ExpressionParser::parseConnectionId(QString strConnectionId, ModbusRegister& modbusReg)
{
    bool bRet = false;

    if (strConnectionId.isEmpty())
    {
        /* Keep default */
        bRet = true;
    }
    else
    {
        quint8 connectionId = static_cast<quint8>(strConnectionId.toUInt(&bRet));
        if (bRet)
        {
            if (connectionId > 1)
            {
                connectionId -= 1;
            }
            modbusReg.setConnectionId(connectionId);
        }
        else
        {
            qCWarning(scopeComm) << QString("Parsing connection \"%1\" failed").arg(strConnectionId);
        }
    }

    return bRet;
}

bool ExpressionParser::parseType(QString strType, ModbusRegister& modbusReg)
{
    bool bRet = true; /* Default to true */

    if (strType == "")
    {
        /* Keep defaults */
    }
    else if (strType == "16b")
    {
        modbusReg.set32Bit(false);
        modbusReg.setUnsigned(true);
    }
    else if (strType == "s16b")
    {
        modbusReg.set32Bit(false);
        modbusReg.setUnsigned(false);
    }
    else if (strType == "32b")
    {
        modbusReg.set32Bit(true);
        modbusReg.setUnsigned(true);
    }
    else if (strType == "s32b")
    {
        modbusReg.set32Bit(true);
        modbusReg.setUnsigned(false);
    }
    else
    {
        qCWarning(scopeComm) << QString("Unknown type \"%1\"").arg(strType);
        bRet = false;
    }

    return bRet;
}

