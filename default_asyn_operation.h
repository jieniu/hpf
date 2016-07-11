/*
 * filename      : default_asyn_operation.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-04 09:20
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _DEFAULT_ASYN_OPERATION_H_
#define _DEFAULT_ASYN_OPERATION_H_

#include "asyn_operation.h"
namespace hpf
{

class DefaultAsynOperation : public AsynOperation
{
    public:
        DefaultAsynOperation(ByteBuffer &bb)
            : AsynOperation(bb)
        {
        }

        virtual ~DefaultAsynOperation()
        {
            delete [] m_byte_buffer.address();
        }

        virtual void on_operation_complete(sint32 result, sint32 reason);
};

inline
void DefaultAsynOperation::on_operation_complete(sint32 result, sint32 reason)
{
    delete this;
}

}
#endif
