#include "expressionparser.h"
#include "expressionregex.h"
#include "modbusdatatype.h"
#include "scopelogging.h"

const QString ExpressionParser::_cRegisterFunctionTemplate = "r(%1%2)";

ExpressionParser::ExpressionParser(QStringList& expressions)
{
    _findRegRegex.setPattern(ExpressionRegex::cMatchRegister);
    _findRegRegex.optimize();

    _regParseRegex.setPattern(ExpressionRegex::cParseReg);
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

    for(const QString &expression: std::as_const(expressions))
    {
        _processedExpressions.append(processExpression(expression));
    }
}

QString ExpressionParser::processExpression(QString const & graphExpr)
{
    QString resultExpr = graphExpr;
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
                QString regFunc = constructInternalRegisterFunction(modbusReg, regDef.size());
                resultExpr.replace(regDef, regFunc);
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

QString ExpressionParser::constructInternalRegisterFunction(ModbusRegister const & modbusReg, int size)
{
    quint32 idx;
    if (_modbusRegisters.contains(modbusReg))
    {
        idx = _modbusRegisters.indexOf(modbusReg);
    }
    else
    {
        _modbusRegisters.append(modbusReg);
        idx = _modbusRegisters.size() - 1;
    }

    /* Add dummy whitespaces to make sure positions in internal representations match visible expressions */
    QString regIdx = QString("%1").arg(idx);
    const int spacesCount = size - 3 - regIdx.size(); /* ignore ${} and idx string length */
    QString spaces = QString(" ").repeated(spacesCount);

    return QString(_cRegisterFunctionTemplate).arg(idx).arg(spaces);
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
        bRet = true;
        modbusReg.setAddress(ModbusAddress(strAddr));
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
            if (connectionId > 0)   /* off by 1 */
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
    bool bRet;
    bool bOk;
    ModbusDataType::Type type = ModbusDataType::convertString(strType, bOk);

    if (bOk)
    {
        modbusReg.setType(type);
        bRet = true;
    }
    else
    {
        bRet = false;
        qCWarning(scopeComm) << QString("Unknown type \"%1\"").arg(strType);
    }

    return bRet;
}

