
#ifndef TEST_GRAPHDATAHANDLER_H__
#define TEST_GRAPHDATAHANDLER_H__

#include "util/result.h"

#include <QObject>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;

class TestGraphDataHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void error_data();
    void error();

    void sameRegisterDifferentType();

    void registerList();
    void manyInactiveRegisters();

    void graphData();
    void graphDataTwice();
    void graphData_fail();

    void graphData_degradedInputPropagatesFlags();
    void graphData_failedEvaluationUnionsInputFlags();
    void graphData_allNoValueInputsYieldsNoValueNotInvalid();
    void graphData_allNoValueInputsKeepFlags();
    void graphData_partialNoValueWithInvalidYieldsInvalid();
    void graphData_constantExpressionIsGoodNoFlags();
    void graphData_degradedInputWithoutFlagsStaysDegraded();
    void graphData_syntaxErrorWithAllNoValueInputsYieldsInvalid();
    void graphData_missingInputResultsYieldsInvalid();
    void graphData_invalidExpressionWithAllNoValueInputsYieldsInvalid();
    void graphData_sharedInputAggregatedPerExpression();
    void graphData_qualityDoesNotLeakBetweenCalls();

private:
    ResultDoubleList doHandleRegisterData(ResultDoubleList modbusResults);

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif /* TEST_GRAPHDATAHANDLER_H__ */
