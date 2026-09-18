
#include "tst_graphdatahandler.h"

#include "../communication/communicationhelpers.h"
#include "datahandling/graphdatahandler.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <QTest>

Q_DECLARE_METATYPE(Result<quint16>);

using State = DataQuality::State;
using Flag = DataQuality::Flag;

void TestGraphDataHandler::init()
{
    qRegisterMetaType<Result<quint16>>("Result<quint16>");
    qRegisterMetaType<QList<Result<quint16>> >("QList<Result<quint16>>");

    _pSettingsModel = new SettingsModel;
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
}

void TestGraphDataHandler::cleanup()
{
    delete _pGraphDataModel;
    delete _pSettingsModel;
}

void TestGraphDataHandler::registerList()
{
    auto exprList = QStringList() << "${40001@2} + ${40001}"
                                  << "${40002:s32b}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expDataPoints = QList<DataPoint>() << DataPoint("${40001@2}", Device::cFirstDeviceId + 1)
                                            << DataPoint("${40001}", Device::cFirstDeviceId)
                                            << DataPoint("${40002:s32b}", Device::cFirstDeviceId);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);

    QCOMPARE(expDataPoints, registerList);
}

void TestGraphDataHandler::error_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<int>("errorPos");
    QTest::addColumn<QMuParser::ErrorType>("errorType");

    QTest::newRow("Test 01") << QString("${40001}") << static_cast<int>(-1) << QMuParser::ErrorType::NONE;
    QTest::newRow("Test 02") << QString("10 + 5") << static_cast<int>(-1) << QMuParser::ErrorType::NONE;
    QTest::newRow("Test 03") << QString("++") << static_cast<int>(2) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 04") << QString("--1") << static_cast<int>(2) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 05") << QString("-1-+-1") << static_cast<int>(5) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 06") << QString("${40001}++") << static_cast<int>(11) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 07") << QString("${40001@1:s16b}++") << static_cast<int>(18) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 08") << QString("${40001@1:s16b}  ++") << static_cast<int>(20) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 09") << QString("${40001 @ 1 }--") << static_cast<int>(16) << QMuParser::ErrorType::SYNTAX;
    QTest::newRow("Test 10") << QString("${40001}\n+1\n+${40001}") << static_cast<int>(-1)
                             << QMuParser::ErrorType::NONE;
}

void TestGraphDataHandler::error()
{
    QFETCH(QString, expression);
    QFETCH(int, errorPos);
    QFETCH(QMuParser::ErrorType, errorType);

    auto exprList = QStringList() << expression;

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good);
    dataHandler.handleRegisterData(regResults);

    QCOMPARE(dataHandler.expressionErrorPos(0), errorPos);
    QCOMPARE(dataHandler.expressionErrorType(0), errorType);
}

void TestGraphDataHandler::sameRegisterDifferentType()
{
    QString expression = QString("${40001@1:s16b}+${40001@1}");

    auto exprList = QStringList() << expression;

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);
    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good) << ResultDouble(1, State::Good);
    dataHandler.handleRegisterData(regResults);

    QCOMPARE(dataHandler.expressionErrorPos(0), -1);
    QCOMPARE(dataHandler.expressionErrorType(0), QMuParser::ErrorType::NONE);
}

void TestGraphDataHandler::manyInactiveRegisters()
{
    /*
     * This test is added to make sure inactive registers are handled correctly
     */

    for (uint32_t idx = 0; idx < 8; idx++)
    {
        _pGraphDataModel->add();
        _pGraphDataModel->setExpression(GraphIdx(idx), "${40001}");
        _pGraphDataModel->setActive(GraphIdx(idx), false);
    }

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(8), "${40002}");

    _pGraphDataModel->add();
    _pGraphDataModel->setExpression(GraphIdx(9), "${40003}");
    _pGraphDataModel->setActive(GraphIdx(9), false);

    auto expDataPoints = QList<DataPoint>() << DataPoint("${40002}", Device::cFirstDeviceId);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);
    QCOMPARE(expDataPoints, registerList);
}

void TestGraphDataHandler::graphData()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good) << ResultDouble(2, State::Good);
    auto expected = ResultDoubleList() << ResultDouble(3, State::Good);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

void TestGraphDataHandler::graphDataTwice()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);

    auto regResults_1 = ResultDoubleList() << ResultDouble(1, State::Good) << ResultDouble(2, State::Good);
    auto result_1 = dataHandler.handleRegisterData(regResults_1);
    QCOMPARE(result_1, ResultDoubleList() << ResultDouble(3, State::Good));

    auto regResults_2 = ResultDoubleList() << ResultDouble(3, State::Good) << ResultDouble(4, State::Good);
    auto result_2 = dataHandler.handleRegisterData(regResults_2);
    QCOMPARE(result_2, ResultDoubleList() << ResultDouble(7, State::Good));
}

void TestGraphDataHandler::graphData_fail()
{
    auto exprList = QStringList() << "${40001} + ${40002}"
                                  << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good) << ResultDouble(0, State::Invalid);
    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid) << ResultDouble(1, State::Good);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief A successfully evaluated expression must be Degraded, not Good, when any contributing
 * data point was Degraded, and must carry the union of the contributing points' flags.
 */
void TestGraphDataHandler::graphData_degradedInputPropagatesFlags()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Degraded, Flag::Substituted)
                                         << ResultDouble(2, State::Good);
    auto expected = ResultDoubleList() << ResultDouble(3, State::Degraded, Flag::Substituted);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief A failed evaluation must still carry the union of the contributing points' flags, not
 * just their state.
 */
void TestGraphDataHandler::graphData_failedEvaluationUnionsInputFlags()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good)
                                         << ResultDouble(0, State::Invalid, Flag::OldData);
    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid, Flag::OldData);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief An expression whose contributing data points are all still NoValue must itself be
 * NoValue ("not started yet"), never Invalid ("broken") — see handleRegisterData().
 */
void TestGraphDataHandler::graphData_allNoValueInputsYieldsNoValueNotInvalid()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(0, State::NoValue) << ResultDouble(0, State::NoValue);
    auto expected = ResultDoubleList() << ResultDouble(0, State::NoValue);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief As soon as one contributing data point has any real state, a failed evaluation is a
 * genuine Invalid again, not NoValue.
 */
void TestGraphDataHandler::graphData_partialNoValueWithInvalidYieldsInvalid()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(0, State::NoValue) << ResultDouble(0, State::Invalid);
    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief A constant expression (no data point references) that evaluates successfully is Good
 * with no flags, since it has no contributing data points to aggregate over.
 */
void TestGraphDataHandler::graphData_constantExpressionIsGoodNoFlags()
{
    auto exprList = QStringList() << "10 + 5";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expected = ResultDoubleList() << ResultDouble(15, State::Good);

    QCOMPARE(doHandleRegisterData(ResultDoubleList()), expected);
}

/*!
 * \brief A Degraded input that carries no recognised flag (the wire allows `state: degraded`
 * without `flags`, and an unknown flag id is dropped on decode) must still make the expression
 * Degraded: the state, not only the flag set, decides.
 */
void TestGraphDataHandler::graphData_degradedInputWithoutFlagsStaysDegraded()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Degraded) << ResultDouble(2, State::Good);
    auto expected = ResultDoubleList() << ResultDouble(3, State::Degraded);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief A genuine syntax error must be reported as Invalid even while every input is still
 * NoValue; only a failure caused by the missing inputs themselves is "not started yet".
 */
void TestGraphDataHandler::graphData_syntaxErrorWithAllNoValueInputsYieldsInvalid()
{
    auto exprList = QStringList() << "${40001}++";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(0, State::NoValue);
    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief An expression that mixes '.' and ',' as decimal separators is malformed and must be
 * Invalid even while every input is still NoValue.
 */
void TestGraphDataHandler::graphData_invalidExpressionWithAllNoValueInputsYieldsInvalid()
{
    auto exprList = QStringList() << "${40001} + 1.5 + 2,5";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(0, State::NoValue);
    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief Fewer input results than the expression references is a protocol problem, not
 * "not started yet": the expression must be Invalid rather than NoValue.
 */
void TestGraphDataHandler::graphData_missingInputResultsYieldsInvalid()
{
    auto exprList = QStringList() << "${40001} + ${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto expected = ResultDoubleList() << ResultDouble(0, State::Invalid);

    QCOMPARE(doHandleRegisterData(ResultDoubleList()), expected);
}

/*!
 * \brief Two expressions sharing one flagged input are aggregated independently: an expression
 * that does not reference the flagged data point must stay Good.
 */
void TestGraphDataHandler::graphData_sharedInputAggregatedPerExpression()
{
    auto exprList = QStringList() << "${40001}"
                                  << "${40001} + ${40002}"
                                  << "${40002}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    auto regResults = ResultDoubleList() << ResultDouble(1, State::Good)
                                         << ResultDouble(2, State::Degraded, Flag::Substituted);
    auto expected = ResultDoubleList() << ResultDouble(1, State::Good)
                                       << ResultDouble(3, State::Degraded, Flag::Substituted)
                                       << ResultDouble(2, State::Degraded, Flag::Substituted);

    QCOMPARE(doHandleRegisterData(regResults), expected);
}

/*!
 * \brief The same handler evaluated again with clean inputs must not carry over the previous
 * cycle's state or flags.
 */
void TestGraphDataHandler::graphData_qualityDoesNotLeakBetweenCalls()
{
    auto exprList = QStringList() << "${40001}";

    CommunicationHelpers::addExpressionsToModel(_pGraphDataModel, exprList);

    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);

    auto flagged = ResultDoubleList() << ResultDouble(1, State::Degraded, Flag::Overflow);
    auto flaggedExpected = ResultDoubleList() << ResultDouble(1, State::Degraded, Flag::Overflow);
    QCOMPARE(dataHandler.handleRegisterData(flagged), flaggedExpected);

    auto clean = ResultDoubleList() << ResultDouble(2, State::Good);
    QCOMPARE(dataHandler.handleRegisterData(clean), ResultDoubleList() << ResultDouble(2, State::Good));
}

ResultDoubleList TestGraphDataHandler::doHandleRegisterData(ResultDoubleList modbusResults)
{
    GraphDataHandler dataHandler;
    QList<DataPoint> registerList;
    dataHandler.setupExpressions(_pGraphDataModel, registerList);
    return dataHandler.handleRegisterData(modbusResults);
}

QTEST_GUILESS_MAIN(TestGraphDataHandler)
