#ifndef COMMUNICATIONHELPERS_H
#define COMMUNICATIONHELPERS_H

#include <QObject>
#include "graphdatamodel.h"

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

    static void verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList)
    {
        /* Verify success */
        QVERIFY((arguments[0].canConvert<QList<bool> >()));
        QList<bool> resultList = arguments[0].value<QList<bool> >();
        QCOMPARE(resultList, expResultList);

        /* Verify values */
        QVERIFY((arguments[1].canConvert<QList<double> >()));
        QList<double> valueList = arguments[1].value<QList<double> >();
        QCOMPARE(valueList, expValueList);
    }


private:

};


#endif // COMMUNICATIONHELPERS_H

