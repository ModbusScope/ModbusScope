#ifndef MODBUSDATATYPE_H
#define MODBUSDATATYPE_H

#include <QObject>

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

    static bool isUnsigned(ModbusDataType::Type type)
    {
        return cDataTypes[static_cast<int>(type)].bUnsigned;
    }

    static bool isFloat(ModbusDataType::Type type)
    {
        return cDataTypes[static_cast<int>(type)].bFloat;
    }

    static QString typeString(ModbusDataType::Type type)
    {
        switch (type)
        {
        case Type::SIGNED_16:
            return "s16b";
            break;
        case Type::UNSIGNED_32:
            return "32b";
            break;
        case Type::SIGNED_32:
            return "s32b";
            break;
        case Type::FLOAT_32:
            return "f32b";
            break;
        case Type::UNSIGNED_16:
        default:
            return "16b";
            break;
        }
    }

    static Type convertSettings(bool is32bit, bool bUnsigned, bool bFloat)
    {
        if (bFloat)
        {
            return Type::FLOAT_32;
        }
        else if (bUnsigned)
        {
            return is32bit ? Type::UNSIGNED_32 : Type::UNSIGNED_16;
        }
        else
        {
            return is32bit ? Type::SIGNED_32 : Type::SIGNED_16;
        }
    }

    static Type convertString(QString strType, bool &bOk)
    {
        bOk = true;

        if (strType == "16b" || strType.isEmpty())
        {
            return Type::UNSIGNED_16;
        }
        else if (strType == "s16b")
        {
            return Type::SIGNED_16;
        }
        else if (strType == "32b")
        {
            return Type::UNSIGNED_32;
        }
        else if (strType == "s32b")
        {
            return Type::SIGNED_32;
        }
        else if (strType == "f32b")
        {
            return Type::FLOAT_32;
        }
        else
        {
            bOk = false;
            return Type::UNSIGNED_16;
        }
    }

    static Type convertMbcString(QString strType, bool &bOk)
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
        bool bUnsigned;
        bool bFloat;
    };

    static const TypeSettings cDataTypes[];
};



#endif // MODBUSDATATYPE_H
