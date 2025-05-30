
#include "communication/modbusregister.h"

#include <QObject>

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
    void singleRegisterFloat32();
    void singleRegisterExplicitCoil();
    void singleRegisterExplicitDiscreteInput();
    void singleRegisterExplicitHolding();
    void singleRegisterExplicitInput();
    void singleRegisterConnType();
    void multiRegisters();
    void multiRegistersDuplicate();
    void failure();
    void failureMulti();
    void combinations();
    void explicitDefaults();
    void sameRegisterDifferentType();
    void spaces();
    void newlines();
    void constant();
    void manyRegisters();

    void verifyParsing(QStringList exprList, QList<ModbusRegister> &expectedRegisters, QStringList &expectedExpression);

private:

};
