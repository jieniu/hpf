#include "worker_thread_pool.h"
#include "worker_thread.h"
#include "base_define.h"
#include <utility>

namespace hpf
{

WorkerThreadPool::WorkerThreadPool()
    : m_thread_pos(0), m_num_active_thread(0)
{
    m_num_active_thread = 0;
    m_thread_pos = 0;
    pthread_mutex_init(&m_init_lock, NULL);
    pthread_mutex_init(&m_thread_pos_mutex, NULL);
    pthread_cond_init(&m_init_cond, NULL);
}

WorkerThreadPool::~WorkerThreadPool()
{
    pthread_mutex_destroy(&m_init_lock);
    pthread_mutex_destroy(&m_thread_pos_mutex);
    pthread_cond_destroy(&m_init_cond);

    for(sint32 i = 0; i < m_num_active_thread; ++i)
    {
        m_worker_threads[i]->stop_routine();
        m_worker_threads[i]->recycle_resource();
    }
}

sint32 WorkerThreadPool::init(sint32 number, timeval &tv)
{
    ret_val_if_fail(number > 0, -1);

    for (sint32 i = 0; i < number; ++i)
    {
        WorkerThread* wt = new WorkerThread(this, i, tv);
        if (-1 == wt->init())
        {
            LOGGER_ERROR(g_framework_logger, "init worker thread failed");
            return -1;
        }
        m_worker_threads.push_back(wt);
    }

    for (sint32 i = 0; i < number; ++i)
    {
        if (-1 == m_worker_threads[i]->start_thread())
        {
            LOGGER_ERROR(g_framework_logger, "start worker thread failed");
            return -1;
        }
    }

    // wait the worker_thread all start work
    pthread_mutex_lock(&m_init_lock);
    while (m_num_active_thread < number)
    {
        pthread_cond_wait(&m_init_cond, &m_init_lock);
    }
    pthread_mutex_unlock(&m_init_lock);

    for (sint32 i = 0; i < number; i++)
    {
        pthread_t id = m_worker_threads[i]->get_thread_id();
        m_thread_ids.push_back(id);
        std::pair<map<pthread_t, sint32>::iterator, bool> res = m_threadid_index_map.insert(std::make_pair<pthread_t, sint32>(id, i));
        if (false == res.second)
        {
            LOGGER_ERROR(g_framework_logger, "insert <threadid, index> fail!");
            return -1;
        }
    }

    LOGGER_INFO(g_framework_logger, "need worker thread number=" << number
            << ", active thread number=" << m_num_active_thread);

    return 0;
}

sint32 WorkerThreadPool::count_active_thread()
{

    pthread_mutex_lock(&m_init_lock);
    m_num_active_thread++;
    pthread_cond_signal(&m_init_cond);
    pthread_mutex_unlock(&m_init_lock);
    return 0;
}

sint32 WorkerThreadPool::dispatch_new_job(Job *job)
{
    uint32 pos = get_thread_pos();
    return dispatch_new_job(pos, job);
}

sint32 WorkerThreadPool::dispatch_new_job(sint32 index, Job *job)
{
    ret_val_if_fail(m_num_active_thread > 0 && index < m_num_active_thread, -1);
    job->set_worker_thread(m_worker_threads[index]);
    if (-1 == m_worker_threads[index]->add_new_job(job))
    {
        LOGGER_ERROR(g_framework_logger, "add_new_job failed, job=" << job);
        return -1;
    }
    return 0;
}

// TODO 可以采用其他的指派方式，例如哈希，或随机，这样就可以去掉锁
sint32 WorkerThreadPool::get_thread_pos()
{
    pthread_mutex_lock(&m_thread_pos_mutex) ;
    uint32 pos = m_thread_pos % m_num_active_thread;
    m_thread_pos++;
    pthread_mutex_unlock(&m_thread_pos_mutex) ;

    return pos;
}


// 用于找到特定的线程在线程池中的位置，未找到则返回-1
sint32 WorkerThreadPool::get_thread_pos(pthread_t id)
{
    map<pthread_t, sint32>::iterator iter = m_threadid_index_map.find(id);
    if (iter != m_threadid_index_map.end())
    {
        return iter->second;
    }
    else
    {
        return -1;
    }
}


sint32 WorkerThreadPool::post_message(MessageHandler *handler, void *arg1, void *arg2)
{
    ret_val_if_fail(m_num_active_thread > 0, -1);

    uint32 pos = get_thread_pos();
    return m_worker_threads[pos]->post_message(handler, arg1, arg2);
}


sint32 WorkerThreadPool::post_message(MessageHandler *handler, void *arg1, void *arg2, pthread_t id)
{
    ret_val_if_fail(m_num_active_thread > 0, -1);
    sint32 pos = get_thread_pos(id);
    ret_val_if_fail(pos != -1, -1);

    return m_worker_threads[pos]->post_message(handler, arg1, arg2);
}


WorkerThread *WorkerThreadPool::get_worker_thread()
{
    sint32 pos = get_thread_pos();
    return m_worker_threads[pos];
}

WorkerThread *WorkerThreadPool::get_worker_thread(pthread_t id)
{
    sint32 pos = get_thread_pos(id);
    if (-1 == pos) return NULL;
    return m_worker_threads[pos];
}
}
