#include "expressionparser.h"

#include "util/expressionregex.h"
#include "util/modbusdatatype.h"
#include "util/scopelogging.h"

const QString ExpressionParser::_cRegisterFunctionTemplate = "r(%1%2)";

ExpressionParser::ExpressionParser(QStringList& expressions)
{
    _findRegRegex.setPattern(ExpressionRegex::cMatchRegister);
    _findRegRegex.optimize();

    _regParseRegex.setPattern(ExpressionRegex::cParseReg);
    _regParseRegex.optimize();

    parseExpressions(expressions);
}

void ExpressionParser::dataPoints(QList<DataPoint>& dataPointList)
{
    dataPointList = _dataPoints;
}

void ExpressionParser::processedExpressions(QStringList& expressionList)
{
    expressionList = _processedExpressions;
}

void ExpressionParser::parseExpressions(QStringList& expressions)
{
    _processedExpressions.clear();
    _dataPoints.clear();

    for (const QString& expression : std::as_const(expressions))
    {
        _processedExpressions.append(processExpression(expression));
    }
}

QString ExpressionParser::processExpression(QString const& graphExpr)
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

            DataPoint dataPoint;
            if (processRegisterExpression(regDef, dataPoint))
            {
                QString regFunc = constructInternalRegisterFunction(dataPoint, regDef.size());
                resultExpr.replace(regDef, regFunc);
            }
        }
    }

    return resultExpr;
}

bool ExpressionParser::processRegisterExpression(QString regExpr, DataPoint& dataPoint)
{
    bool bRet = false;

    QRegularExpressionMatch match = _regParseRegex.match(regExpr);

    QString strAddress;
    QString strDeviceId;
    QString strType;

    if (match.hasMatch())
    {
        strAddress = match.captured(1);
        strDeviceId = match.captured(2);
        strType = match.captured(3);

        bRet = true;
        bRet = bRet && parseAddress(strAddress, dataPoint);
        bRet = bRet && parseDeviceId(strDeviceId, dataPoint);
        bRet = bRet && parseType(strType, dataPoint);
    }
    else
    {
        auto msg = QString("Part of expression evaluation parsing failed (\"%1\")").arg(regExpr);
        qCWarning(scopeComm) << msg;
        bRet = false;
    }

    return bRet;
}

QString ExpressionParser::constructInternalRegisterFunction(DataPoint const& dataPoint, int size)
{
    quint32 idx;
    if (_dataPoints.contains(dataPoint))
    {
        idx = _dataPoints.indexOf(dataPoint);
    }
    else
    {
        _dataPoints.append(dataPoint);
        idx = _dataPoints.size() - 1;
    }

    /* Add dummy whitespaces to make sure positions in internal representations match visible expressions */
    QString regIdx = QString("%1").arg(idx);
    const int spacesCount = size - 3 - regIdx.size(); /* ignore ${} and idx string length */
    QString spaces = QString(" ").repeated(spacesCount);

    return QString(_cRegisterFunctionTemplate).arg(idx).arg(spaces);
}

bool ExpressionParser::parseAddress(QString strAddr, DataPoint& dataPoint)
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
        dataPoint.setAddress(strAddr);
    }

    return bRet;
}

bool ExpressionParser::parseDeviceId(QString strDeviceId, DataPoint& dataPoint)
{
    bool bRet = false;

    if (strDeviceId.isEmpty())
    {
        /* Keep default */
        bRet = true;
    }
    else
    {
        auto deviceId = strDeviceId.toUInt(&bRet);
        if (bRet)
        {
            dataPoint.setDeviceId(deviceId);
        }
        else
        {
            qCWarning(scopeComm) << QString("Parsing device \"%1\" failed").arg(strDeviceId);
        }
    }

    return bRet;
}

bool ExpressionParser::parseType(QString strType, DataPoint& dataPoint)
{
    bool bRet;
    bool bOk;
    ModbusDataType::Type type = ModbusDataType::convertString(strType, bOk);

    if (bOk)
    {
        dataPoint.setType(type);
        bRet = true;
    }
    else
    {
        bRet = false;
        qCWarning(scopeComm) << QString("Unknown type \"%1\"").arg(strType);
    }

    return bRet;
}
