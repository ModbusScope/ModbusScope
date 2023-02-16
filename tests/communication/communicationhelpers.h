#ifndef COMMUNICATIONHELPERS_H
#define COMMUNICATIONHELPERS_H

#include <QObject>
#include "graphdatamodel.h"
#include "qtestcase.h"
#include "result.h"

class CommunicationHelpers : public QObject
{
    Q_OBJECT

public:

    static void addExpressionsToModel(GraphDataModel* pModel, QStringList &exprList)
    {
        for(const QString &expr: qAsConst(exprList))
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
        QCOMPARE(resultList, actResultList);
    }


private:

};


#endif // COMMUNICATIONHELPERS_H

