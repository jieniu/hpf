#include "event_thread.h"
#include <fcntl.h>
#include "job_message_handler.h"
#include "global_var.h"

namespace hpf
{

AddJobHandler EventThread::s_add_job_handler;
UpdateJobHandler EventThread::s_update_job_handler;
DeleteJobHandler EventThread::s_delete_job_handler;

class StopThreadHandler : public MessageHandler
{
    public:
        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            EventThread *event_thread = (EventThread *)arg1;
            event_thread->do_stop_routine();
        }
};

StopThreadHandler EventThread::s_stop_thread_handler;

EventThread::EventThread()
    : Job(-1)
{
}

EventThread::~EventThread()
{
    if (m_message_fd > 0)
    {
        close(m_message_fd);
        m_message_fd = -1;
    }
    if (m_fd > 0)
    {
        close(m_fd);
        m_fd = -1;
    }
}
        
/*
 * Function Descript : 初始化，线程会监听一个事件，外部可以抛消息给该线程处理
 * Create Time       : 2011-08-03 20:36
 * Parameter List    : 
 * 
 * Return            : 
 *  -1: 失败
 *  0: 成功
 * Modify Time       : 
 */
sint32 EventThread::init()
{
    if (-1 == m_event_base.init_event_base())
    {
        LOGGER_ERROR(g_framework_logger, "init_event_base failed.");
        return -1;
    }

    // create pipe
    sint32 fd[2];
    if (-1 == pipe(fd))
    {
        LOGGER_ERROR(g_framework_logger, "create pipe error: " << strerror(errno));
        return -1;
    }
    m_fd = fd[0];
    m_message_fd = fd[1];

    int flag = fcntl(m_message_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    int ret = fcntl(m_message_fd, F_SETFL, flag);
    if (ret != 0)
    {
        LOGGER_ERROR(g_framework_logger, "set pipe nonblock error:" << strerror(errno));
        return -1;
    }
    flag = fcntl(m_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    ret = fcntl(m_fd, F_SETFL, flag);
    if (ret != 0)
    {
        LOGGER_ERROR(g_framework_logger, "set pipe nonblock error:" << strerror(errno));
        return -1;
    }

    // add a pipe event
    m_event_flags = EV_READ | EV_PERSIST;
    if (-1 == m_event_base.add_new_event(this))
    {
        LOGGER_ERROR(g_framework_logger, "add_new_event error.");
        return -1;
    }

    return self_init();
}

sint32 EventThread::start_thread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t thread_id = 0;
    if (0 != pthread_create(&thread_id, &attr, &thread_routine, this))
    {
        LOGGER_ERROR(g_framework_logger, "pthread_create error: " << strerror(errno));
        return -1;
    }
    return 0;
}

/*
 * Function Descript : 线程执行函数，执行事件循环
 * Create Time       : 2011-08-03 20:38
 * Parameter List    : 
 *  1. arg: 就是this
 * Return            : 
 * Modify Time       : 
 */
void* EventThread::thread_routine(void* arg)
{
    LOGGER_TRACE(g_framework_logger, "event thread start!!!");
    EventThread *et = (EventThread*)arg;
    et->m_thread_id = pthread_self();

    et->begin_routine();
    et->m_event_base.event_loop();
    LOGGER_TRACE(g_framework_logger, "event thread exit!!!");
    pthread_exit((void*)0);
}
        
/*
 * Function Descript : 收到一个外部抛进来的消息，并进行处理
 * Create Time       : 2011-08-03 20:40
 * Parameter List    : 
 *  1. fd: 触发消息的句柄
 *  2. which: 事件类型
 * Return            : 
 *  0: 成功
 *  -1: 失败
 * Modify Time       : 
 */
sint32 EventThread::run(sint32 fd, sint16 which)
{
    assert(m_fd == fd);
    unsigned char buf;
    while (1 == read(fd, &buf, sizeof(buf)))
    {}
    if (errno != EAGAIN && errno != EINTR)
    {
        LOGGER_ERROR(g_framework_logger, "event thread read pipe failed.");
        return -1;
    }

    EventMessage em;
    while (-1 != m_message_queue.pop(em))
    {
        em.handle_message(get_no());
//        delete em;
    }

    return 0;
}

/*
 * Function Descript : 抛一个消息给线程
 * Create Time       : 2011-08-03 20:41
 * Parameter List    : 
 *  1. handler: 处理消息的回调类
 *  2. arg1: 消息的参数，该参数会传给处理函数
 *  3. arg2: 消息的参数，该参数会传给处理函数
 * Return            : 
 * Modify Time       : 
 */
sint32 EventThread::post_message(MessageHandler *handler, void *arg1, void *arg2)
{
    if (NULL == handler)
    {
        return -1;
    }
    EventMessage em(handler, arg1, arg2);
    m_message_queue.push_back(em);

    unsigned char buf = 1;
    if (sizeof(buf) != write(m_message_fd, &buf, sizeof(buf)) && errno != EAGAIN)
    {
        LOGGER_ERROR(g_framework_logger, "write to thread's pipe error: " << strerror(errno));
        m_message_queue.erase_latest(em);
        return -1;
    }

    return 0;
}
 
sint32 EventThread::send_message(MessageHandler *handler, void *arg1, void *arg2)
{
    if (NULL == handler)
    {
        return -1;
    }
    if (pthread_equal(m_thread_id, pthread_self()))
    {
        LOGGER_TRACE(g_framework_logger, "send message to the same thread:" << m_thread_id);
        handler->process_message(get_no(), arg1, arg2);
        return 0;
    }
    return post_message(handler, arg1, arg2);
}

// 线程安全
sint32 EventThread::add_new_job(Job *job)
{
    return post_message(&s_add_job_handler, this, job);
}

// 线程不安全，不是本线程不能直接被调用
sint32 EventThread::do_add_job(Job *job)
{
    LOGGER_DEBUG(g_framework_logger, "add job: " << job << ", event_base: " << &m_event_base);
    sint32 ret = m_event_base.add_new_event(job);
    return ret;
}

// 线程安全
sint32 EventThread::update_job(Job *job)
{
    return post_message(&s_update_job_handler, this, job);
}

// 线程不安全，不是本线程不能直接被调用
sint32 EventThread::do_update_job(Job *job)
{
    return m_event_base.update_event(job);
}

// 线程安全
sint32 EventThread::delete_job(Job *job)
{
    return post_message(&s_delete_job_handler, this, job);
}

// 线程不安全，不是本线程不能直接被调用
sint32 EventThread::do_delete_job(Job *job)
{
    LOGGER_DEBUG(g_framework_logger, "delete job: " << job << ", event_base: " << &m_event_base);
    sint32 ret = m_event_base.delete_event(job);
    job->delete_event_success();
    return ret;
}

/*
 * Function Descript : 外部通过该函数停止事件循环
 * Create Time       : 2011-08-03 20:24
 * Parameter List    : 
 * Return            : 
 * Modify Time       : 
 */
sint32 EventThread::stop_routine()
{
    post_message(&s_stop_thread_handler, this, NULL);    
    LOGGER_INFO(g_framework_logger, "wait for thread end, thread_id=" << m_thread_id);
    return 0;
}

// 线程不安全，不是本线程不能直接被调用
sint32 EventThread::do_stop_routine()
{
    LOGGER_DEBUG(g_framework_logger, "do_stop_routine!!!");
    do_delete_job(this);

    end_routine();
    sint32 ret = m_event_base.event_loop_exit();
    LOGGER_DEBUG(g_framework_logger, "event_loop_exit:" << ret);
    return ret;
}

sint32 EventThread::recycle_resource()
{
    void *tret = 0;
    pthread_join(m_thread_id, &tret);
    LOGGER_INFO(g_framework_logger, "thread end success, thread_id=" << m_thread_id
            << ", exit_no=" << (size_t)tret);

    delete this;
    return 0;
}

}
