
#include "modbusdatatype.h"

const ModbusDataType::TypeSettings ModbusDataType::cDataTypes[] =
{
    /* UNSIGNED_16 */ {false, true,  false},
    /* SIGNED_16   */ {false, false, false},
    /* UNSIGNED_32 */ {true,  true,  false},
    /* SIGNED_32   */ {true,  false, false},
    /* FLOAT_32    */ {true,  false, true },
};
