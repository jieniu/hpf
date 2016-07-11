/*
 * filename      : listen_thread.h
 * descriptor    :  ¼àÌýÏß³Ì
 * author        : fengyajie
 * create time   : 2010-12-15 19:43
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _LISTEN_THREAD_H_
#define _LISTEN_THREAD_H_

//#include "base_header.h"
#include "event_thread.h"
#include "global_var.h"

namespace hpf
{

class WorkerThreadPool;
class MessageProcessor;

enum ConnectType
{
    LONG_CONNECT,
    SHORT_CONNECT,
};


class ListenJob;
class ListenThread : public EventThread
{
    public:
        ListenThread(const char *ip, uint16 port, WorkerThreadPool *thread_pool,
                MessageProcessor *msg_proc, sint32 timeout);
        virtual ~ListenThread();

        virtual sint32 self_init();

        virtual sint32 begin_routine() {return 0;}

        virtual sint32 end_routine();

        virtual sint32 add_new_connect(sint32 fd) = 0;

    protected:
        string m_host_ip;
        uint16 m_host_port;
        WorkerThreadPool *m_thread_pool;
        MessageProcessor *m_message_processor;
        sint32 m_connect_timeout;
        ListenJob *m_listen_job;
};

}
#endif
