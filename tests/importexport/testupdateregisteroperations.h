
#include <QObject>

#include "projectfiledata.h"

class TestUpdateRegisterOperations: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void noOperation();

    void bitmask();
    void shiftLeft();
    void shiftRight();
    void multiply();
    void divide();

    void allOperations();
    void bitmaskShift();
    void bitmaskMultiply();
    void shiftDivide();
    void multiplyDivide();

private:
    void checkOperation(ProjectFileData::RegisterSettings& regSettings, QString regResult);

};
