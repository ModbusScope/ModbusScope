#include "modbusdatatype.h"

const ModbusDataType::TypeSettings ModbusDataType::cDataTypes[] =
{
    /* UNSIGNED_16 */ {false},
    /* SIGNED_16   */ {false},
    /* UNSIGNED_32 */ {true},
    /* SIGNED_32   */ {true},
    /* FLOAT_32    */ {true},
};
