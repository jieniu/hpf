/*
 * filename      : worker_thread_pool.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-16 23:16
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * | 8.4            | fengyajie     | 整理头文件                |
 * +----------------+---------------+---------------------------+
 */
#ifndef _WORKER_THREAD_POOL_H_
#define _WORKER_THREAD_POOL_H_
#include "base_header.h"

namespace hpf
{

class Job;
class WorkerThread;
class MessageHandler;

class WorkerThreadPool
{
    public:
        WorkerThreadPool();
        ~WorkerThreadPool();
        typedef vector<WorkerThread*> WorkerThreads;
        typedef WorkerThreads::iterator IterWorkerThreads;

        sint32 init(sint32 number, timeval &tv);

        sint32 dispatch_new_job(Job *job);
        sint32 dispatch_new_job(sint32 index, Job *job);

        sint32 count_active_thread();

        sint32 get_thread_pos();
        sint32 get_thread_pos(pthread_t id);

        sint32 get_wthread_number() {return m_num_active_thread;}

        sint32 post_message(MessageHandler *handler, void *arg1, void *arg2);
        sint32 post_message(MessageHandler *handler, void *arg1, void *arg2, pthread_t id);

        WorkerThread *get_worker_thread();
        WorkerThread *get_worker_thread(pthread_t id);

        const vector<pthread_t> &get_thread_ids() {return m_thread_ids;}


    private:
        pthread_mutex_t m_init_lock;
        pthread_cond_t m_init_cond;
        pthread_mutex_t m_thread_pos_mutex;

        uint32 m_thread_pos;
        sint32 m_num_active_thread;
        WorkerThreads m_worker_threads;
        vector<pthread_t> m_thread_ids;

        // 用于获取threadid线程对应在pool中的位置
        map<pthread_t, sint32> m_threadid_index_map;
};

}
#endif
