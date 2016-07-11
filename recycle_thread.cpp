#include "recycle_thread.h"
#include "event_message.h"
#include "global_var.h"

namespace hpf
{

class RecycleHandler : public MessageHandler
{
    virtual void process_message(sint32 no, void *arg1, void *arg2)
    {
        RecycleThread *rt = (RecycleThread*)arg1;
        rt->do_recycle_thread((EventThread*)arg2);
    }
};

RecycleThread *RecycleThread::s_instance = NULL;
RecycleHandler RecycleThread::s_recycle_handler;

RecycleThread::RecycleThread()
    : m_is_start(false)
{
    pthread_mutex_init(&m_init_lock, NULL);
    pthread_cond_init(&m_init_cond, NULL);
}
        
RecycleThread::~RecycleThread()
{
    pthread_mutex_destroy(&m_init_lock);
    pthread_cond_destroy(&m_init_cond);
}
        
RecycleThread *RecycleThread::get_instance()
{
    return s_instance;
}

RecycleThread *RecycleThread::create_instance()
{
    if (s_instance == NULL)
    {
        s_instance = new RecycleThread();
        s_instance->init();
        s_instance->start_thread();
        return s_instance;
    }

    return s_instance;
}

void RecycleThread::destroy()
{
    stop_routine();
    recycle_resource();
}

// 线程初始化时调用
sint32 RecycleThread::self_init()
{
    return 0;
}

// 线程进入事件循环前调用
sint32 RecycleThread::begin_routine()
{
    pthread_mutex_lock(&m_init_lock);
    m_is_start = true;
    pthread_cond_signal(&m_init_cond);
    pthread_mutex_unlock(&m_init_lock);
    LOGGER_INFO(g_framework_logger, "start recycle thread success.");
    return 0;
}

// 线程停止事件循环前调用
sint32 RecycleThread::end_routine()
{
    return 0;
}

sint32 RecycleThread::recycle_thread(EventThread *et)
{
    if (NULL == et)
    {
        return -1;
    }
    return post_message(&s_recycle_handler, this, et);
}

sint32 RecycleThread::do_recycle_thread(EventThread *et)
{
    if (-1 == et->stop_routine())
    {
        return -1;
    }

    return et->recycle_resource();
}


void RecycleThread::wait_for_start()
{

    pthread_mutex_lock(&m_init_lock);
    while (!m_is_start)
    {
        pthread_cond_wait(&m_init_cond, &m_init_lock);
    }
    pthread_mutex_unlock(&m_init_lock);
}

}
