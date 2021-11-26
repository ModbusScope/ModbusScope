
#include <QObject>

class TestQMuParser: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void evaluate_data();
    void evaluate();

    void evaluateSingleRegister_data();
    void evaluateSingleRegister();

    void evaluateMultipleRegisters();

    void evaluateInvalidExpr();
    void evaluateInvalidDecimal();
    void evaluateDivByZero();

    void evaluateInvalidHexExpr();
    void evaluateInvalidBinExpr();
    void evaluateInvalidBinExpr_2();

    void evaluateDecimalSeparatorCombination();
    void expressionGet();
    void expressionUpdate();

private:


};
