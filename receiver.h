/*
 * filename      : receiver.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 00:28
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _RECEIVER_H_
#define _RECEIVER_H_
#include "base_header.h"
#include "bytebuffer/bytebuffer.h"

namespace hpf
{

class MessageProcessor;

class Receiver
{
    const static sint32 DEFAULT_RECV_LENGTH = 1024;
    const static sint32 MAX_RECV_LENGTH = 1024 * 1024 * 16;
    const static sint32 MAX_MALLOC_ONCE = 4;

    public:
        Receiver(MessageProcessor *mp)
            : m_message_processor(mp)
        {}
        virtual ~Receiver()
        {
            clean();
        }

        sint32 read_data(sint32 fd);

        virtual void clean();

        virtual sint32 process_recv(sint32 handle) = 0;

    protected:
        ByteBuffer m_recv_buffer; // 接收缓存
        sint32 m_recv_length;
        MessageProcessor *m_message_processor;
};

}
#endif
