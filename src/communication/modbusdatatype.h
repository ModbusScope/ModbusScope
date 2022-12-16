#ifndef MODBUSDATATYPE_H
#define MODBUSDATATYPE_H

#include <QObject>

class ModbusDataType
{

public:
    enum Type
    {
        UNSIGNED_16,
        SIGNED_16,
        UNSIGNED_32,
        SIGNED_32,
        FLOAT_32,
    };

    static bool is32Bit(ModbusDataType::Type type)
    {
        return cDataTypes[type].b32Bit;
    }

    static bool isUnsigned(ModbusDataType::Type type)
    {
        return cDataTypes[type].bUnsigned;
    }

    static bool isFloat(ModbusDataType::Type type)
    {
        return cDataTypes[type].bFloat;
    }

    static Type convertString(QString strType, bool &bOk)
    {
        bOk = true;

        if (strType == "16b" || strType.isEmpty())
        {
            return UNSIGNED_16;
        }
        else if (strType == "s16b")
        {
            return SIGNED_16;
        }
        else if (strType == "32b")
        {
            return UNSIGNED_32;
        }
        else if (strType == "s32b")
        {
            return SIGNED_32;
        }
        else if (strType == "f32b")
        {
            return FLOAT_32;
        }
        else
        {
            bOk = false;
            return UNSIGNED_16;
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
