
#include "modbusdatatype.h"

const ModbusDataType::TypeSettings ModbusDataType::cDataTypes[] =
{
    [static_cast<int>(ModbusDataType::Type::UNSIGNED_16)] = {.b32Bit = false, .bUnsigned = true, .bFloat = false},
    [static_cast<int>(ModbusDataType::Type::SIGNED_16)] = {.b32Bit = false, .bUnsigned = false, .bFloat = false},
    [static_cast<int>(ModbusDataType::Type::UNSIGNED_32)] = {.b32Bit = true, .bUnsigned = true, .bFloat = false},
    [static_cast<int>(ModbusDataType::Type::SIGNED_32)] = {.b32Bit = true, .bUnsigned = false, .bFloat = false},
    [static_cast<int>(ModbusDataType::Type::FLOAT_32)] = {.b32Bit = true, .bUnsigned = false, .bFloat = true},
};
