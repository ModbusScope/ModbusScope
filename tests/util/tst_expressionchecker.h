
#include <QObject>

class TestExpressionChecker: public QObject
{
    Q_OBJECT
    
private slots:
    void init();
    void cleanup();

    void dataIsPrimed();
    void expressionIsValid();
    void expressionHasSyntaxError();
    void valuErrorIsNotSyntaxError();

private:

};
