
#ifndef TEST_READREGISTERS_H__
#define TEST_READREGISTERS_H__

#include <QObject>

/* Forward declaration */
class ReadRegisters;

#include "modbusaddress.h"

class TestReadRegisters: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void resetRead_1();
    void resetRead_2();
    void resetReadSplit_1();
    void resetReadSplit_2();

    void resetReadDifferentObjectTypes_1();
    void resetReadDifferentObjectTypes_2();

    void consecutive_1();
    void consecutive_2();
    void consecutive_3();

    void splitNextToSingleReads_1();
    void splitNextToSingleReads_2();
    void splitNextToSingleReads_3();

    void addAllErrors();
    void addSuccess();
    void addSuccessAndErrors();

private:

    void verifyAndAddErrorResult(ReadRegisters& readRegister, ModbusAddress addr, quint16 cnt);

};

#endif /* TEST_READREGISTERS_H__ */
