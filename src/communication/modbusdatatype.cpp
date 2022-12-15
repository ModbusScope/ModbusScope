
#include "modbusdatatype.h"

const ModbusDataType::TypeSettings ModbusDataType::cDataTypes[] =
{
    [ModbusDataType::UNSIGNED_16] = {.b32Bit = false, .bUnsigned = true, .bFloat = false},
    [ModbusDataType::SIGNED_16] = {.b32Bit = false, .bUnsigned = false, .bFloat = false},
    [ModbusDataType::UNSIGNED_32] = {.b32Bit = true, .bUnsigned = true, .bFloat = false},
    [ModbusDataType::SIGNED_32] = {.b32Bit = true, .bUnsigned = false, .bFloat = false},
    [ModbusDataType::FLOAT_32] = {.b32Bit = true, .bUnsigned = false, .bFloat = true},
};