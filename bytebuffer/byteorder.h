#ifndef _MYBASE_BYTEORDER_H_
#define _MYBASE_BYTEORDER_H_

#include "./bits.h"

class ByteOrder
{
public:
    string name;
public:
    ByteOrder(string name) ;

    static ByteOrder big_endian;
    static ByteOrder little_endian;

    static ByteOrder nativeOrder();
};

#endif

