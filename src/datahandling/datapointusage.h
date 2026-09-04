#ifndef DATAPOINTUSAGE_H
#define DATAPOINTUSAGE_H

#include "communication/datapoint.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class GraphDataModel;
class SettingsModel;

/*!
 * \brief Determines which data points the active signals actually use.
 *
 * The adapter enforces its own limit on the number of data points a session may read, so the
 * counts reported here are the same ones the adapter checks: the deduplicated data points of
 * the active signals, grouped by the adapter that owns their device.
 */
namespace DataPointUsage {

QStringList activeExpressions(const GraphDataModel* pGraphDataModel);

QList<DataPoint> activeDataPoints(const GraphDataModel* pGraphDataModel);

QMap<QString, int> countPerAdapter(const QList<DataPoint>& dataPoints, SettingsModel* pSettingsModel);

} // namespace DataPointUsage

#endif // DATAPOINTUSAGE_H
