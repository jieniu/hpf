/*
 * filename      : hcc_receiver.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 00:48
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HCC_RECEIVER_H_
#define _HCC_RECEIVER_H_
#include "receiver.h"
#include "message_processor.h"

namespace hpf
{

class HccReceiver : public Receiver
{
    public:
        HccReceiver(MessageProcessor *mp)
            : Receiver(mp)
        {}
        virtual ~HccReceiver(){}


        virtual sint32 process_recv(sint32 handle);
};

/*
 * Function Descript : 分包及处理包
 * Create Time       : 2011-08-24 01:02
 * Parameter List    : 
 *  0: 正常
 *  -1: 出错，需要断开连接
 * Return            : 
 * Modify Time       : 
 */
inline sint32 HccReceiver::process_recv(sint32 handle)
{
    m_recv_buffer.flip();

    // TODO: user need to move the position
    if (-1 == m_message_processor->process_input(&m_recv_buffer, handle))
    {
        return -1;
    }

    return 0;
}

}
#endif
