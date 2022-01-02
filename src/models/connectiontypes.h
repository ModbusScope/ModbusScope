#ifndef CONNECTION_TYPES_H
#define CONNECTION_TYPES_H

namespace Connection
{
    enum
    {
        ID_1 = 0,
        ID_2,
        ID_3,
        ID_CNT
    };

    typedef enum
    {
        TYPE_TCP = 0,
        TYPE_SERIAL,
        TYPE_CNT
    } type_t;

}

#endif // CONNECTION_TYPES_H
