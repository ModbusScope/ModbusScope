
#ifndef TEST_GRAPHDATAHANDLER_H__
#define TEST_GRAPHDATAHANDLER_H__

#include <QObject>

#include "settingsmodel.h"
#include "result.h"

/* Forward declaration */
class GraphDataModel;

class TestGraphDataHandler: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void registerList();
    void manyInactiveRegisters();

    void graphData();
    void graphData_fail();

private:

    void doHandleRegisterData(QList<Result>& modbusResults, QList<QVariant> &actRawData);
    void addExpressionsToModel(GraphDataModel* pModel, QStringList &exprList);
    void verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList);

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};

#endif /* TEST_GRAPHDATAHANDLER_H__ */
