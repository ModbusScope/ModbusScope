#include "datapointusage.h"

#include "datahandling/expressionparser.h"
#include "models/adapterdata.h"
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
QMap<QString, int> countPerAdapter(const QList<DataPoint>& dataPoints, const SettingsModel* pSettingsModel)
{
    QMap<QString, int> countByAdapter;
    for (const DataPoint& dataPoint : dataPoints)
    {
        ++countByAdapter[pSettingsModel->adapterIdForDevice(dataPoint.deviceId())];
    }

    return countByAdapter;
}

/*!
 * \brief Describe every adapter that is asked to read more data points than it currently allows.
 *
 * The limit is the adapter's reported capabilities.maxRegisters, which is only present while the
 * adapter enforces a cap (for example when no valid license is found).
 * \param dataPoints The data points to check, typically from activeDataPoints().
 * \param pSettingsModel Model resolving the adapter of a device and its limit.
 * \return One line per adapter over its limit, ordered by adapter id; empty when every adapter is
 *         within its limit.
 */
QStringList limitWarnings(const QList<DataPoint>& dataPoints, SettingsModel* pSettingsModel)
{
    const QMap<QString, int> countByAdapter = countPerAdapter(dataPoints, pSettingsModel);

    /* adapterData() inserts a default entry for an unknown adapterId, which would leave a phantom
       entry behind for a warning alone. An adapter SettingsModel has never heard of cannot have
       reported a limit either, so there is nothing to warn about. */
    const QStringList knownAdapterIds = pSettingsModel->adapterIds();

    QStringList warnings;
    for (auto it = countByAdapter.constBegin(); it != countByAdapter.constEnd(); ++it)
    {
        if (!knownAdapterIds.contains(it.key()))
        {
            continue;
        }

        const int dataPointLimit = pSettingsModel->adapterData(it.key())->maxRegisters();
        if (it.value() > dataPointLimit)
        {
            warnings.append(QString("%1 allows at most %2 data point(s), but %3 are configured.")
                              .arg(pSettingsModel->adapterDisplayName(it.key()))
                              .arg(dataPointLimit)
                              .arg(it.value()));
        }
    }

    return warnings;
}

} // namespace DataPointUsage
