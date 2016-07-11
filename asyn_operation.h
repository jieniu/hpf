/*
 * filename      : asyn_operation.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-04 09:12
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _ASYN_OPERATION_H_
#define _ASYN_OPERATION_H_
#include "bytebuffer/bytebuffer.h"
namespace hpf
{

class AsynOperation
{
    public:
        AsynOperation(ByteBuffer &bb)
            : m_byte_buffer(bb)
        {}
        virtual ~AsynOperation(){}


        ByteBuffer& get_bytebuffer() {return m_byte_buffer;}

        virtual void on_operation_complete(sint32 result, sint32 reason) = 0;
    protected:
        ByteBuffer m_byte_buffer;
};

}
#endif
