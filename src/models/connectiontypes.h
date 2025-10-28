#ifndef CONNECTION_TYPES_H
#define CONNECTION_TYPES_H

#include <qtypes.h>

namespace ConnectionTypes {

using connectionId_t = quint32;

typedef enum
{
    ID_1 = 0,
    ID_2,
    ID_3,
    ID_CNT
} id_t;

typedef enum
{
    TYPE_TCP = 0,
    TYPE_SERIAL,
    TYPE_CNT
} type_t;

} // namespace ConnectionTypes

#endif // CONNECTION_TYPES_H
