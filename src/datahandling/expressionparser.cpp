#include "expressionparser.h"

#include "models/device.h"
#include "util/expressionregex.h"
#include "util/scopelogging.h"

const QString ExpressionParser::_cRegisterFunctionTemplate = "r(%1%2)";

ExpressionParser::ExpressionParser(QStringList& expressions)
{
    _findRegRegex.setPattern(ExpressionRegex::cMatchRegister);
    _findRegRegex.optimize();

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
    static const QRegularExpression regParseRegex(ExpressionRegex::cParseReg);
    QRegularExpressionMatch match = regParseRegex.match(regExpr);

    if (!match.hasMatch())
    {
        qCWarning(scopeComm) << QString("Part of expression evaluation parsing failed (\"%1\")").arg(regExpr);
        return false;
    }

    const QString strDeviceId = match.captured(2);

    deviceId_t deviceId = Device::cFirstDeviceId;
    if (!strDeviceId.isEmpty())
    {
        bool ok;
        deviceId = strDeviceId.toUInt(&ok);
        if (!ok)
        {
            qCWarning(scopeComm) << QString("Parsing device \"%1\" failed").arg(strDeviceId);
            return false;
        }
    }

    dataPoint = DataPoint(regExpr, deviceId);
    return true;
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
