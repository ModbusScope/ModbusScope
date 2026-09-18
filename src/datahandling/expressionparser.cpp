#include "expressionparser.h"

#include "models/device.h"
#include "util/expressionregex.h"
#include "util/scopelogging.h"

const QString ExpressionParser::_cRegisterFunctionTemplate = "r(%1%2)";

ExpressionParser::ExpressionParser(const QStringList& expressions) : _findRegRegex(ExpressionRegex::cMatchRegister)
{
    parseExpressions(expressions);
}

/*!
 * \brief Returns the deduplicated list of data points found during parsing.
 */
QList<DataPoint> ExpressionParser::dataPoints() const
{
    return _dataPoints;
}

/*!
 * \brief Returns the list of processed expressions with data point references replaced.
 */
QStringList ExpressionParser::processedExpressions() const
{
    return _processedExpressions;
}

/*!
 * \brief Returns, for each expression, the indices into dataPoints() it references.
 *
 * One entry per expression passed to the constructor, in the same order. A data point referenced by
 * more than one expression appears in each of their index lists; an expression with no data point
 * references (a constant expression) has an empty list. Needed to aggregate data quality over an
 * expression's inputs even when the expression itself fails to evaluate.
 */
QList<QList<int>> ExpressionParser::expressionDataPointIndices() const
{
    return _expressionDataPointIndices;
}

void ExpressionParser::parseExpressions(const QStringList& expressions)
{
    _processedExpressions.clear();
    _dataPoints.clear();
    _expressionDataPointIndices.clear();

    for (const QString& expression : expressions)
    {
        QList<int> dataPointIndices;
        _processedExpressions.append(processExpression(expression, dataPointIndices));
        _expressionDataPointIndices.append(dataPointIndices);
    }
}

QString ExpressionParser::processExpression(QString const& graphExpr, QList<int>& dataPointIndices)
{
    QString resultExpr = graphExpr;
    QRegularExpressionMatchIterator i = _findRegRegex.globalMatch(resultExpr);

    if (!i.hasNext() && resultExpr.contains("$"))
    {
        qCWarning(scopeComm) << qUtf8Printable(
          QString("Expression evaluation parsing failed (\"%1\")").arg(resultExpr));
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
                QString regFunc = constructInternalRegisterFunction(dataPoint, regDef.size(), dataPointIndices);
                resultExpr.replace(regDef, regFunc);
            }
        }
    }

    return resultExpr;
}

bool ExpressionParser::processRegisterExpression(const QString& regExpr, DataPoint& dataPoint)
{
    static const QRegularExpression regParseRegex(ExpressionRegex::cParseReg);
    QRegularExpressionMatch match = regParseRegex.match(regExpr);

    if (!match.hasMatch())
    {
        qCWarning(scopeComm) << qUtf8Printable(
          QString("Part of expression evaluation parsing failed (\"%1\")").arg(regExpr));
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
            qCWarning(scopeComm) << qUtf8Printable(QString("Parsing device \"%1\" failed").arg(strDeviceId));
            return false;
        }
    }

    dataPoint = DataPoint(regExpr, deviceId);
    return true;
}

QString ExpressionParser::constructInternalRegisterFunction(DataPoint const& dataPoint,
                                                            int size,
                                                            QList<int>& dataPointIndices)
{
    qsizetype idx = _dataPoints.indexOf(dataPoint);
    if (idx < 0)
    {
        _dataPoints.append(dataPoint);
        idx = _dataPoints.size() - 1;
    }
    dataPointIndices.append(static_cast<int>(idx));

    /* Add dummy whitespaces to make sure positions in internal representations match visible expressions */
    QString regIdx = QString::number(idx);
    const int spacesCount = qMax(0, size - 3 - regIdx.size()); /* ignore ${} and idx string length */
    QString spaces = QString(" ").repeated(spacesCount);

    return QString(_cRegisterFunctionTemplate).arg(idx).arg(spaces);
}
