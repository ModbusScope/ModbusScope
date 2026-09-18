#include "graphdatahandler.h"

#include "datahandling/datapointusage.h"
#include "datahandling/expressionparser.h"
#include "datahandling/qmuparser.h"
#include "models/graphdatamodel.h"
#include "util/scopelogging.h"

#include <QRegularExpression>

/*!
 * \param[in]     pGraphDataModel   Graph data model
 * \param[out]    registerList      List of data points
 */
void GraphDataHandler::setupExpressions(GraphDataModel* pGraphDataModel, QList<DataPoint>& registerList)
{
    pGraphDataModel->activeGraphIndexList(_activeIndexList);

    ExpressionParser exprParser(DataPointUsage::activeExpressions(pGraphDataModel));
    const QList<DataPoint> regList = exprParser.dataPoints();
    const QStringList processedExpList = exprParser.processedExpressions();
    _expressionDataPointIndices = exprParser.expressionDataPointIndices();

    _valueParsers.clear();

    for (const QString& expr : std::as_const(processedExpList))
    {
        _valueParsers.append(QMuParser(expr));
    }

    registerList = regList;
}

QString GraphDataHandler::expressionParseMsg(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QString();
    }

    return _valueParsers[exprIdx].msg();
}

qint32 GraphDataHandler::expressionErrorPos(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return -1;
    }

    return _valueParsers[exprIdx].errorPos();
}

QMuParser::ErrorType GraphDataHandler::expressionErrorType(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QMuParser::ErrorType::SYNTAX;
    }

    return _valueParsers[exprIdx].errorType();
}

/*!
 * \brief Evaluates each configured expression against the raw data point results.
 *
 * State and flags are aggregated across the expression's contributing data points: a successful
 * evaluation is Degraded when any input was Degraded (whether or not it carried flags), otherwise
 * Good; a failed evaluation is Invalid, unless every contributing input is still NoValue and the
 * expression itself is well-formed, in which case it is NoValue too ("not started yet" rather than
 * "broken", so the gap before the first read is not reported as a failure). An input result that
 * is missing altogether counts as a real fault, not as NoValue. Flags are the union of the
 * contributing inputs' flags on both paths; every referenced data point contributes, even one an
 * expression such as if() does not end up evaluating. A constant expression (no data point
 * references) that evaluates successfully is Good with no flags.
 *
 * \param results Raw data point read results from the adapter (one entry per data point).
 * \return Expression-evaluated results (one entry per graph expression). The input values are
 *         not passed through, only their aggregated quality and the expression's own value.
 */
ResultDoubleList GraphDataHandler::handleRegisterData(const ResultDoubleList& results)
{
    ResultDoubleList registerList;

    QMuParser::setRegistersData(results);

    for (qsizetype exprIdx = 0; exprIdx < _valueParsers.size(); exprIdx++)
    {
        QMuParser& parser = _valueParsers[exprIdx];
        const QList<int> dataPointIndices =
          (exprIdx < _expressionDataPointIndices.size()) ? _expressionDataPointIndices[exprIdx] : QList<int>();

        DataQuality::Flags flags = DataQuality::Flag::NoFlags;
        bool anyDegraded = false;
        bool allNoValue = !dataPointIndices.isEmpty();
        for (int dataPointIdx : dataPointIndices)
        {
            if (dataPointIdx < 0 || dataPointIdx >= results.size())
            {
                allNoValue = false;
                continue;
            }
            flags |= results[dataPointIdx].flags();
            if (results[dataPointIdx].state() == DataQuality::State::Degraded)
            {
                anyDegraded = true;
            }
            if (results[dataPointIdx].state() != DataQuality::State::NoValue)
            {
                allNoValue = false;
            }
        }

        ResultDouble result;

        const bool evaluated = parser.evaluate();
        /* Only a failure caused by the inputs themselves counts as "not started yet"; a malformed
         * expression is a fault even while every input is still NoValue. */
        const bool inputsUnavailable = allNoValue && (parser.errorType() == QMuParser::ErrorType::OTHER);

        if (evaluated)
        {
            result.setValue(parser.value());
            result.addFlags(flags);
            if (anyDegraded)
            {
                result.setState(DataQuality::State::Degraded);
            }
        }
        else if (!inputsUnavailable)
        {
            result.setError();
            result.addFlags(flags);

            auto msg = QString("Expression evaluation failed (%1)").arg(parser.msg());

            qCWarning(scopeComm) << qUtf8Printable(msg);
        }
        /* else: every contributing data point is still NoValue and the expression itself is fine, so
         * the result stays at its default NoValue state; not an evaluation failure worth logging. */

        registerList.append(result);
    }

    return registerList;
}
