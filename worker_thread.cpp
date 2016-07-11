#include "worker_thread.h"
#include "event_base.h"
#include "worker_thread_pool.h"
#include "base_define.h"

namespace hpf
{
/*
 * Function Descript : 构造函数
 * Create Time       : 2011-08-20 18:29
 * Parameter List    : 
 *  1. tp: 线程池
 *  2. no: 线程id（非pthread_self） 
 *  3. tv: 统计任务的间隔时间
 * Return            : 
 * Modify Time       : 
 */
WorkerThread::WorkerThread(WorkerThreadPool *tp, sint32 no, timeval &tv)
    : m_thread_pool(tp), m_no(no)
{
    m_tv.tv_sec = tv.tv_sec;
    m_tv.tv_usec = tv.tv_usec;
}

WorkerThread::~WorkerThread()
{
}

sint32 WorkerThread::self_init()
{
    ret_val_if_fail(m_thread_pool != NULL, -1);

    LOGGER_INFO(g_framework_logger, "worker thread init success, no=" << m_no
            << ", thread_id=" << pthread_self());
    return 0;
}

sint32 WorkerThread::begin_routine()
{
    LOGGER_INFO(g_framework_logger, "worker thread start routine, no=" << m_no);
    return m_thread_pool->count_active_thread();
}


sint32 WorkerThread::end_routine() 
{
    LOGGER_INFO(g_framework_logger, "worker thread end routine, no=" << m_no);
    return 0;
}

}
