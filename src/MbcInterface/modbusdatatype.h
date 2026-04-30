#ifndef MODBUSDATATYPE_H
#define MODBUSDATATYPE_H

#include <QString>

class ModbusDataType
{

public:

    enum class Type
    {
        UNSIGNED_16 = 0,
        SIGNED_16 = 1,
        UNSIGNED_32 = 2,
        SIGNED_32 = 3,
        FLOAT_32 = 4,
    };

    static bool is32Bit(ModbusDataType::Type type)
    {
        return cDataTypes[static_cast<int>(type)].b32Bit;
    }

    static QString typeString(ModbusDataType::Type type)
    {
        switch (type)
        {
        case Type::SIGNED_16:
            return "s16b";
        case Type::UNSIGNED_32:
            return "32b";
        case Type::SIGNED_32:
            return "s32b";
        case Type::FLOAT_32:
            return "f32b";
        case Type::UNSIGNED_16:
        default:
            return "16b";
        }
    }

    static Type convertMbcString(QString strType, bool& bOk)
    {
        bOk = true;

        if (
            (strType == "uint16")
            || (strType == "hex16")
            || (strType == "bin16")
            || (strType == "ascii16")
            || strType.isEmpty()
        )
        {
            return Type::UNSIGNED_16;
        }
        else if (strType == "int16")
        {
            return Type::SIGNED_16;
        }
        else if (
            (strType == "uint32")
            || (strType == "hex32")
            || (strType == "bin32")
            || (strType == "ascii32")
        )
        {
            return Type::UNSIGNED_32;
        }
        else if (strType == "int32")
        {
            return Type::SIGNED_32;
        }
        else if (strType == "float32")
        {
            return Type::FLOAT_32;
        }
        else
        {
            bOk = false;
            return Type::UNSIGNED_16;
        }
    }

private:

    struct TypeSettings
    {
        bool b32Bit;
    };

    static const TypeSettings cDataTypes[];
};



#endif // MODBUSDATATYPE_H
