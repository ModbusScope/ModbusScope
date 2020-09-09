
#include <QObject>

class TestQMuParser: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void evaluate_data();
    void evaluate();

    void evaluateRegister_data();
    void evaluateRegister();

    void evaluateInvalidExpr();
    void evaluateInvalidDecimal();
    void evaluateDivByZero();

    void expressionGet();
    void expressionUpdate();

private:


};
