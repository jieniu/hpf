/*
 * filename      : recycle_thread.h
 * descriptor    : 专门用来回收线程的线程
 * author        : fengyajie
 * create time   : 2011-08-27 15:41
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _RECYCLE_THREAD_H_
#define _RECYCLE_THREAD_H_
#include "event_thread.h"

namespace hpf
{

class RecycleHandler;
class RecycleThread : public EventThread
{
    public:
        sint32 recycle_thread(EventThread *et);

        sint32 do_recycle_thread(EventThread *et);

        void destroy();
        
        static RecycleThread *get_instance();
        
        static RecycleThread *create_instance();

        // 线程初始化时调用
        virtual sint32 self_init();

        // 线程进入事件循环前调用
        virtual sint32 begin_routine();

        // 线程停止事件循环前调用
        virtual sint32 end_routine();

        void wait_for_start();
        
    private:
        RecycleThread();
        virtual ~RecycleThread();

        static RecycleHandler s_recycle_handler;
        static RecycleThread *s_instance;
        
        bool m_is_start;
        pthread_mutex_t m_init_lock;
        pthread_cond_t m_init_cond;
};

}
#endif
