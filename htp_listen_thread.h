/*
 * filename      : htp_listen_thread.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-23 23:43
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_LISTEN_THREAD_H_
#define _HTP_LISTEN_THREAD_H_

#include "listen_thread.h"

namespace hpf
{

class MessageParser;
class HtpListenThread : public ListenThread
{
    public:
        HtpListenThread(const char *ip, uint16 port, WorkerThreadPool *thread_pool,
                MessageParser *parser, MessageProcessor *msg_proc, sint32 timeout)
            : ListenThread(ip, port, thread_pool, msg_proc, timeout)
        {
            m_parser = parser;
        }
        virtual ~HtpListenThread()
        {}

        virtual sint32 add_new_connect(sint32 fd);
    private:
        MessageParser *m_parser;
};

}
#endif
