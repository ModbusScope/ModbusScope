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
