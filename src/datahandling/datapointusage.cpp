#include "datapointusage.h"

#include "datahandling/expressionparser.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <utility>

namespace DataPointUsage {

/*!
 * \brief Collect the expressions of every active signal.
 * \param pGraphDataModel Model holding the configured signals.
 * \return The expressions, in model order, of the signals that are active.
 */
QStringList activeExpressions(const GraphDataModel* pGraphDataModel)
{
    QList<GraphIdx> activeIndexList;
    pGraphDataModel->activeGraphIndexList(activeIndexList);

    QStringList expressions;
    for (GraphIdx graphIdx : std::as_const(activeIndexList))
    {
        expressions.append(pGraphDataModel->expression(graphIdx));
    }

    return expressions;
}

/*!
 * \brief Collect the data points referenced by the active signals.
 * \param pGraphDataModel Model holding the configured signals.
 * \return The deduplicated data points, exactly as they are requested from the adapters when
 *         logging starts. A data point used by several signals is counted once.
 */
QList<DataPoint> activeDataPoints(const GraphDataModel* pGraphDataModel)
{
    ExpressionParser exprParser(activeExpressions(pGraphDataModel));
    return exprParser.dataPoints();
}

/*!
 * \brief Count data points per adapter.
 * \param dataPoints The data points to count, typically from activeDataPoints().
 * \param pSettingsModel Model resolving the adapter that owns a device.
 * \return The number of data points per adapter id. Adapters without data points are absent.
 */
QMap<QString, int> countPerAdapter(const QList<DataPoint>& dataPoints, SettingsModel* pSettingsModel)
{
    QMap<QString, int> countByAdapter;
    for (const DataPoint& dataPoint : dataPoints)
    {
        ++countByAdapter[pSettingsModel->adapterIdForDevice(dataPoint.deviceId())];
    }

    return countByAdapter;
}

} // namespace DataPointUsage
