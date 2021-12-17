
#include <QObject>
#include "modbusregister.h"

class TestExpressionParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleRegister();
    void singleRegisterConn();
    void singleRegisterSigned();
    void singleRegisterSigned32();
    void singleRegisterConnType();
    void multiRegisters();
    void multiRegistersDuplicate();
    void failure();
    void failureMulti();
    void combinations();
    void spaces();
    void constant();

    void verifyParsing(QStringList exprList, QList<ModbusRegister> &expectedRegisters, QStringList &expectedExpression);

private:

};
