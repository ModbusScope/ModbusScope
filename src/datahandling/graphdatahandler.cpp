#include "graphdatahandler.h"

#include "datahandling/datapointusage.h"
#include "datahandling/expressionparser.h"
#include "datahandling/qmuparser.h"
#include "models/graphdatamodel.h"
#include "util/scopelogging.h"

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

    for (const QString& expr : processedExpList)
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

namespace {

//! Quality of the data points one expression references, combined into what its result needs.
struct InputSummary
{
    DataQuality::Flags flags = DataQuality::Flag::NoFlags;
    bool anyDegraded = false;
    bool allNoValue = false;
};

/*!
 * \brief Combines the quality of the data points an expression references.
 *
 * Flags are the union over every referenced data point, even one an expression such as if() does
 * not end up evaluating. A data point whose result is missing altogether is a real fault, so it
 * prevents allNoValue. A constant expression (no references) is never allNoValue.
 *
 * \param dataPointIndices Indices into \a results of the data points the expression references.
 * \param results Raw data point read results from the adapter.
 */
InputSummary summarizeInputs(const QList<int>& dataPointIndices, const ResultDoubleList& results)
{
    InputSummary summary;
    summary.allNoValue = !dataPointIndices.isEmpty();

    for (int dataPointIdx : dataPointIndices)
    {
        if (dataPointIdx < 0 || dataPointIdx >= results.size())
        {
            summary.allNoValue = false;
            continue;
        }

        const ResultDouble& input = results[dataPointIdx];
        summary.flags |= input.flags();

        if (input.state() == DataQuality::State::Degraded)
        {
            summary.anyDegraded = true;
        }
        if (input.state() != DataQuality::State::NoValue)
        {
            summary.allNoValue = false;
        }
    }

    return summary;
}

/*!
 * \brief Evaluates one expression and derives the quality of its result from its inputs.
 *
 * A successful evaluation is Degraded when any input was Degraded (whether or not it carried
 * flags), otherwise Good. A failed evaluation is NoValue when every input is still NoValue and the
 * failure came from the inputs rather than from the expression itself ("not started yet" rather
 * than "broken"); any other failure is Invalid. The inputs' flags are carried over on every path.
 *
 * \param parser The expression to evaluate.
 * \param inputs Combined quality of the data points the expression references.
 */
ResultDouble evaluateExpression(QMuParser& parser, const InputSummary& inputs)
{
    ResultDouble result; /* Defaults to NoValue */

    if (parser.evaluate())
    {
        result.setValue(parser.value());
        result.addFlags(inputs.flags);
        if (inputs.anyDegraded)
        {
            result.setState(DataQuality::State::Degraded);
        }
    }
    else if (inputs.allNoValue && parser.errorType() == QMuParser::ErrorType::OTHER)
    {
        /* Not an evaluation failure worth logging. A malformed expression (any other error type)
         * is a fault even while every input is still NoValue. */
        result.addFlags(inputs.flags);
    }
    else
    {
        result.setError();
        result.addFlags(inputs.flags);

        auto msg = QString("Expression evaluation failed (%1)").arg(parser.msg());

        qCWarning(scopeComm) << qUtf8Printable(msg);
    }

    return result;
}

} // namespace

/*!
 * \brief Evaluates each configured expression against the raw data point results.
 *
 * The quality of each result is aggregated from the data points its expression references.
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
        const InputSummary inputs = summarizeInputs(_expressionDataPointIndices.at(exprIdx), results);
        registerList.append(evaluateExpression(_valueParsers[exprIdx], inputs));
    }

    return registerList;
}
