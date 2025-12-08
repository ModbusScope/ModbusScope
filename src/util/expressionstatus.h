#ifndef EXPRESSION_STATUS_H
#define EXPRESSION_STATUS_H

#include <QObject>
#include "expressionchecker.h"

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
    void handlExpressionsChanged(const quint32 graphIdx);

private:
    void verifyExpression(QString const& expression, QList<deviceId_t> const& deviceIdList);

    ExpressionChecker _checker;

    GraphDataModel* _pGraphDataModel;
    SettingsModel* _pSettingsModel;

    QQueue<QString> _expressionQueue;

};

#endif // EXPRESSION_STATUS_H
