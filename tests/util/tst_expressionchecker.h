
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
    void valueErrorIsNotSyntaxError();

    void checkForDevices_allPresent();
    void checkForDevices_missing();

private:

};
