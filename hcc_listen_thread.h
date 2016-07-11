/*
 * filename      : hcc_listen_thread.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-23 23:07
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HCC_LISTEN_THREAD_H_
#define _HCC_LISTEN_THREAD_H_

#include "listen_thread.h"

namespace hpf
{

class HccListenThread : public ListenThread
{
    public:
        HccListenThread(const char *ip, uint16 port, WorkerThreadPool *thread_pool,
                        MessageProcessor *msg_proc, sint32 timeout)
                        : ListenThread(ip, port, thread_pool, msg_proc, timeout)
        {}
        virtual ~HccListenThread()
        {}

        virtual sint32 add_new_connect(sint32 fd);
};

}
#endif
