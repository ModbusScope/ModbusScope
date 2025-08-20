#ifndef COMMUNICATIONHELPERS_H
#define COMMUNICATIONHELPERS_H

#include "models/graphdatamodel.h"
#include "util/result.h"

#include <QObject>
#include <qtestcase.h>

class CommunicationHelpers : public QObject
{
    Q_OBJECT

public:

    static void addExpressionsToModel(GraphDataModel* pModel, QStringList &exprList)
    {
        for(const QString &expr: std::as_const(exprList))
        {
            pModel->add();
            pModel->setExpression(pModel->size() - 1, expr);
        }
    }

    static void verifyReceivedDataSignal(QList<QVariant> arguments, ResultDoubleList resultList)
    {
        /* Verify success */
        QVERIFY(arguments[0].canConvert<ResultDoubleList>());
        auto actResultList = arguments[0].value<ResultDoubleList>();
        QCOMPARE(actResultList, resultList);
    }


private:

};


#endif // COMMUNICATIONHELPERS_H

