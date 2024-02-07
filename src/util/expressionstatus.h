#ifndef EXPRESSION_STATUS_H
#define EXPRESSION_STATUS_H

#include <QObject>
#include "expressionchecker.h"

class GraphDataModel;

class ExpressionStatus : public QObject
{
    Q_OBJECT

public:
    explicit ExpressionStatus(GraphDataModel *pGraphDataModel, QObject *parent = nullptr);

private slots:
    void handleResultReady(bool valid);
    void handlExpressionsChanged(const quint32 graphIdx);

private:

    void verifyExpression(QString &expression);

    ExpressionChecker _checker;

    GraphDataModel* _pGraphDataModel;

    QQueue<QString> _expressionQueue;

};

#endif // EXPRESSION_STATUS_H
