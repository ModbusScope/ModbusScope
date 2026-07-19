#include "mbcdatatype.h"

const MbcDataType::TypeSettings MbcDataType::cDataTypes[] =
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
 * \brief Byte length of a string type such as "string50" (0 when not a sized string).
 * \param strType The string representation of the type.
 * \return The byte length, or 0 if not a sized string.
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
