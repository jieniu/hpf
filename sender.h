/*
 * filename      : sender.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 09:18
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _SENDER_H_
#define _SENDER_H_

#include "base_header.h"
#include "fifo.h"

namespace hpf
{

class AsynOperation;

class Sender
{
    public:
        typedef Fifo<AsynOperation *> SendBufferList;

        Sender()
        {}
        virtual ~Sender() {}

        sint32 write_data(sint32 fd, AsynOperation *ao, sint32 &write_len);

        sint32 do_write_data(sint32 fd, AsynOperation *ao, sint32 &write_len);

        sint32 flush_data(sint32 fd, sint32 &flush_data);

        sint32 get_ao_count() {return m_send_buffer_list.size();}

        virtual void clean() = 0;

    protected:
        SendBufferList m_send_buffer_list;  // 发送缓存
};

}
#endif
