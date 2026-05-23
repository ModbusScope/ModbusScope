#ifndef EXPRESSION_STATUS_H
#define EXPRESSION_STATUS_H

#include "expressionchecker.h"
#include "util/graphindex.h"
#include <QObject>
#include <QPair>
#include <QQueue>

/* Forward declarations */
class GraphDataModel;
class SettingsModel;

class ExpressionStatus : public QObject
{
    Q_OBJECT

public:
    explicit ExpressionStatus(GraphDataModel* pGraphDataModel,
                              SettingsModel* pSettingsModel,
                              QObject* parent = nullptr);

private slots:
    void handleResultReady(bool valid);
    void handleExpressionsChanged(GraphIdx graphIdx);

private:
    void verifyExpression(QString const& expression, QList<deviceId_t> const& deviceIdList);

    ExpressionChecker _checker;
    bool _deviceCheckPassed;

    GraphDataModel* _pGraphDataModel;
    SettingsModel* _pSettingsModel;

    QQueue<QPair<GraphIdx, QString>> _expressionQueue;
};

#endif // EXPRESSION_STATUS_H
