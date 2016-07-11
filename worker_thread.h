/*
 * filename      : worker_thread.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-16 22:45
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include "base_header.h"
#include "event_thread.h"

namespace hpf
{

class WorkerThreadPool;

class WorkerThread : public EventThread
{
    public:
        WorkerThread(WorkerThreadPool *tp, sint32 no, timeval &tv);
        virtual ~WorkerThread();
        
        virtual sint32 self_init();

        virtual sint32 begin_routine();

        virtual sint32 end_routine();

        virtual sint32 get_no() {return m_no;}

        virtual pthread_t get_thread_id() {return m_thread_id;}
    private:
        WorkerThreadPool *m_thread_pool;
        sint32 m_no; // 内部id
        timeval m_tv;
};

}
#endif
