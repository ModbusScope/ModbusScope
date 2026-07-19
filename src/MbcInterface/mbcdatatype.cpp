#include "mbcdatatype.h"

const MbcDataType::TypeSettings MbcDataType::cDataTypes[MbcDataType::cTypeCount] =
{
    /*                   32-bit  supported */
    /* UNSIGNED_16 */ {  false,  true      },
    /* SIGNED_16   */ {  false,  true      },
    /* UNSIGNED_32 */ {  true,   true      },
    /* SIGNED_32   */ {  true,   true      },
    /* FLOAT_32    */ {  true,   true      },
    /* STRING      */ {  false,  false     },
};

/*!
 * \brief Whether ModbusScope can plot a register of this type.
 */
bool MbcDataType::isSupported(MbcDataType::Type type)
{
    return cDataTypes[static_cast<int>(type)].bSupported;
}

/*!
 * \brief Byte length of a string type such as "string50" (0 when not a sized string).
 */
quint32 MbcDataType::stringByteLength(const QString& strType)
{
    if (!strType.startsWith("string"))
    {
        return 0;
    }

    bool bOk = false;
    const quint32 bytes = strType.mid(6).toUInt(&bOk);
    return bOk ? bytes : 0;
}
