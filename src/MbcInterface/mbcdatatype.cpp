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
