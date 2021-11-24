#include "expressionparser.h"

#include "scopelogging.h"

const QString ExpressionParser::_cRegisterPattern = "\\${(\\d?.*?)}";
const QString ExpressionParser::_cParseRegPattern = "(\\d+)(?:@(\\d+))?(?:\\:(\\w+))?";

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

    if (!i.hasNext())
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
                if (!_modbusRegisters.contains(modbusReg))
                {
                    _modbusRegisters.append(modbusReg);
                }
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

        if (!strAddress.isEmpty())
        {
            bool bRet;
            quint16 addr = static_cast<quint16>(strAddress.toUInt(&bRet));
            if (bRet)
            {
                modbusReg.setAddress(addr);
            }
            else
            {
                qCWarning(scopeComm) << QString("Parsing address \"%1\" failed").arg(strAddress);
            }
        }

        if (!strConnectionId.isEmpty())
        {
            bool bRet;
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

        if (!strType.isEmpty())
        {
            /* TODO: change to switch of all supported types */
            if (
                (strType == "s32b")
                || (strType == "s16b")
            )
            {
                modbusReg.setUnsigned(false);
            }

            if (
                (strType == "32b")
                || (strType == "s32b")
            )
            {
                modbusReg.set32Bit(true);
            }
        }

        bRet = true;
    }
    else
    {
        auto msg = QString("Part of expression evaluation parsing failed (\"%1\")").arg(regExpr);
        qCWarning(scopeComm) << msg;
        bRet = false;
    }

    return bRet;
}
