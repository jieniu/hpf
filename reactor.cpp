#include "reactor.h"
#include "listen_thread.h"
#include "worker_thread_pool.h"
#include "hcc_connection_info.h"
#include "SDSocketUtility.h"
#include "worker_thread.h"
#include "hcc_listen_thread.h"
#include "recycle_thread.h"
#include "htp_connection_info.h"
#include "htp_listen_thread.h"
#include "udp_server.h"
#include "timeout_job.h"

namespace hpf
{

Reactor *Reactor::s_instance = NULL;

Reactor::Reactor()
   : m_worker_thread_pool(0), m_seqid(0)
{
    pthread_mutex_init(&m_mutex, NULL);
//    m_http_request = NULL;
}

Reactor::~Reactor()
{
    for (uint32 i = 0; i < m_listen_threads.size(); ++i)
    {
        m_listen_threads[i]->stop_routine();
        m_listen_threads[i]->recycle_resource();
    }
    m_listen_threads.clear();

    delete m_worker_thread_pool;
    m_worker_thread_pool = NULL;

    pthread_mutex_destroy(&m_mutex);
}

/*
 * Function Descript : 初始化：创建线程池, 并启动其中线程, 创建回收线程, TODO 线程数是静态的
 * Create Time       : 2011-07-29 09:30
 * Parameter List    :
 * 1. sint32 num_thread : 线程池中的线程数
 * 2. tv: 工作线程统计间隔时间
 * Return            :
 * Modify Time       :
 */
sint32 Reactor::init(sint32 num_thread, sint32 sec)
{
    ret_val_if_fail(num_thread >= 1, -1);

    m_worker_thread_pool = new WorkerThreadPool();
    timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    ret_val_if_fail(-1 != m_worker_thread_pool->init(num_thread, tv), -1);

    RecycleThread::create_instance();

    LOGGER_INFO(g_framework_logger, "reactor init success.");
    return 0;
}


/*
 * Function Descript : 添加一个高并发监听端口
 * Create Time       : 2011-08-02 09:11
 * Parameter List    :
 *  1. ip: 本地ip
 *  2. port: 本地端口
 *  3. mp: 处理数据的接口
 *  4. timeout: 超时时间
 * Return            :
 *  0: 成功
 *  -1: 失败
 * Modify Time       :
 */
sint32 Reactor::add_hcc_listener(const char *ip, uint16 port,
        MessageProcessor *mp, sint32 timeout)
{
    HccListenThread *listen_thread = new HccListenThread(ip, port,
            m_worker_thread_pool, mp, timeout);
    ret_val_if_fail(listen_thread, -1);
    if (-1 == listen_thread->init())
    {
        goto ERR_EXIT;
    }

    if (-1 == listen_thread->start_thread())
    {
        goto ERR_EXIT;
    }
    m_listen_threads.push_back(listen_thread);

    LOGGER_INFO(g_framework_logger, "add_hcc_listener success, ip=" << ip
            << ", port=" << port << ", connect_timeout=" << timeout);
    return 0;

ERR_EXIT:
    delete listen_thread;
    LOGGER_ERROR(g_framework_logger, "init listen thread failed.");

    return -1;
}

/*
 * Function Descript : 添加一个普通连接
 * Create Time       : 2011-08-02 09:22
 * Parameter List    :
 *
 * Return            :
 * Modify Time       :
 */
sint32 Reactor::add_connector(const char *ip, uint16 port,
        MessageProcessor *mp, bool is_reconnect)
{
    ret_val_if_fail(m_worker_thread_pool != NULL, -1);

    struct sockaddr_in si;
    sint32 fd = SDSocketUtility::nonblock_connect_to(ip, port, &si);
    if (-1 == fd && is_reconnect)
    {
        HccConnectionInfo *ci = new HccConnectionInfo(-1, ip, port, mp, is_reconnect, 0);
        LOGGER_DEBUG(g_framework_logger, "connect failed and reconnect later: " << ip << ":" << port);
        ci->add_reconnect_timeout();
        return 0;
    }
    else if (-1 == fd)
    {
        LOGGER_ERROR(g_framework_logger, "connect failed, dest=" << ip << ":" << port);
        return -1;
    }

    HccConnectionInfo *ci = new HccConnectionInfo(fd, ip, port, mp, is_reconnect, 0);

    // if writable then the connection is connected completely
    // if writable and readable then error happend
    ci->set_event_flags(EV_WRITE | EV_READ | EV_PERSIST);
    if(m_worker_thread_pool->dispatch_new_job(ci) != 0)
    {
        LOGGER_ERROR(g_framework_logger, "add connector failed: add event error.");
        delete ci;
        return -1;
    }

    return 0;
}


/*
 * Function Descript : 增加一个超时事件
 * Create Time       : 2011-08-04 14:28
 * Parameter List    :
 *  1. th: 超时处理对象
 *  2. arg: 参数
 *  3. tv: 超时时间
 *  4. rotate: 是否循环 1=循环 0=不循环
 *  5. handle: 句柄, 范围是非0的无符号整数，32位
 * Return            :
 *  0: 成功
 *  -1: 出错
 * Modify Time       :
 */
sint32 Reactor::add_timer(TimeoutHandler *th, void *arg, const timeval &tv,
                          sint32 rotate, uint32 &handle)
{
    ret_val_if_fail(m_worker_thread_pool, -1);
    ret_val_if_fail((tv.tv_sec > 0 || tv.tv_usec > 0), -1);
    ret_val_if_fail(th != 0, -1)

    pthread_mutex_lock(&m_mutex);
    m_seqid++;
    while (0 == m_seqid || 0 != m_timers.count(m_seqid))
    {
        m_seqid++;
    }

    TimeoutJob *tj = new TimeoutJob(th, arg, tv, rotate, m_seqid);
    assert(tj);
    m_timers.insert(std::make_pair(m_seqid, tj));

    if (-1 == m_worker_thread_pool->dispatch_new_job(tj))
    {
        delete tj;
        m_timers.erase(m_seqid);
        pthread_mutex_unlock(&m_mutex);
        LOGGER_ERROR(g_framework_logger, "dispatch_new_job failed.");
        return -1;
    }
    handle = m_seqid;
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

sint32 Reactor::add_timer(TimeoutHandler *th, uint32 arg, const timeval &tv,
                          sint32 rotate, uint32 &handle)
{
    ret_val_if_fail(m_worker_thread_pool, -1);
    ret_val_if_fail((tv.tv_sec > 0 || tv.tv_usec > 0), -1);
    ret_val_if_fail(th != 0, -1)

    pthread_mutex_lock(&m_mutex);
    m_seqid++;
    while (0 == m_seqid || 0 != m_timers.count(m_seqid))
    {
        m_seqid++;
    }

    TimeoutJobUint *tj = new TimeoutJobUint(th, arg, tv, rotate, m_seqid);
    assert(tj);
    m_timers.insert(std::make_pair(m_seqid, tj));

    if (-1 == m_worker_thread_pool->dispatch_new_job(tj))
    {
        delete tj;
        m_timers.erase(m_seqid);
        pthread_mutex_unlock(&m_mutex);
        LOGGER_ERROR(g_framework_logger, "dispatch_new_job failed.");
        return -1;
    }
    handle = m_seqid;
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

void Reactor::cancel_timer(sint32 handle)
{
    pthread_mutex_lock(&m_mutex);
    Timers::iterator it = m_timers.find(handle);
    if (it == m_timers.end())
    {
        pthread_mutex_unlock(&m_mutex);
        return;
    }

    it->second->get_worker_thread()->delete_job(it->second);
    m_timers.erase(it);

    pthread_mutex_unlock(&m_mutex);
}

sint32 Reactor::create_udp_server(const char *ip, uint16 port, MessageProcessor *mp)
{
    UdpServer *udpserver = new UdpServer(ip, port);
    assert(udpserver != NULL);
    if (udpserver->init(m_worker_thread_pool, mp) < 0)
    {
        LOGGER_ERROR(g_framework_logger, "init udp server error");
        delete udpserver;
        return -1;
    }
    return 0;
}

Reactor *Reactor::get_instance()
{
    if (s_instance)
    {
        return s_instance;
    }

    s_instance = new Reactor();

    return s_instance;
}


void Reactor::destroy()
{
    if (RecycleThread::get_instance())
    {
        RecycleThread::get_instance()->destroy();
    }

    delete this;
    s_instance = NULL;
}

/*
 * Function Descript : 高并发连接
 * Create Time       : 2011-08-25 09:19
 * Parameter List    :
 *  1. ip: 对端ip
 *  2. port: 对端port
 *  3. parser: 消息分包接口
 *  4. mp: 消息处理接口
 *  5. is_reconnect: 是否重连
 * Return            :
 *  1. 0 = succ
 *  2. -1 = fail
 * Modify Time       :
 */
sint32 Reactor::add_htp_connector(const char *ip, uint16 port, MessageParser *parser,
                MessageProcessor *mp, sint32 is_reconnect)
{

    ret_val_if_fail(m_worker_thread_pool != NULL, -1);

    struct sockaddr_in si;
    sint32 fd = SDSocketUtility::nonblock_connect_to(ip, port, &si);
    if (-1 == fd && is_reconnect)
    {
        HtpConnectionInfo *ci = new HtpConnectionInfo(-1, ip, port, parser, mp, is_reconnect, 0);
        ret_val_if_fail(ci != NULL, -1);
        LOGGER_DEBUG(g_framework_logger, "connect failed and reconnect later: " << ip
                     << ":" << port);
        return ci->add_reconnect_timeout();
    }
    else if (-1 == fd)
    {
        LOGGER_ERROR(g_framework_logger, "connect failed, dest=" << ip << ":" << port);
        return -1;
    }

    HtpConnectionInfo *ci = new HtpConnectionInfo(fd, ip, port, parser, mp, is_reconnect, 0);
    ret_val_if_fail(ci->init(false) == 0, -1);
    return 0;
}

sint32 Reactor::add_htp_listener(const char *ip, uint16 port, MessageParser *parser,
                MessageProcessor *mp, sint32 timeout)
{
    HtpListenThread *listen_thread = new HtpListenThread(ip, port, m_worker_thread_pool,
            parser, mp, timeout);
    ret_val_if_fail(listen_thread, -1);
    if (-1 == listen_thread->init())
    {
        goto ERR_EXIT;
    }

    if (-1 == listen_thread->start_thread())
    {
        goto ERR_EXIT;
    }
    m_listen_threads.push_back(listen_thread);

    LOGGER_INFO(g_framework_logger, "add_htp_listener success, ip=" << ip
            << ", port=" << port << ", connect_timeout=" << timeout);
    return 0;

ERR_EXIT:
    delete listen_thread;
    LOGGER_ERROR(g_framework_logger, "init listen thread failed.");

    return -1;
}

sint32 Reactor::post_message(MessageHandler *handler, void *arg1, void *arg2)
{
    ret_val_if_fail(m_worker_thread_pool != NULL, -1);

    return m_worker_thread_pool->post_message(handler, arg1, arg2);
}


sint32 Reactor::post_message(MessageHandler *handler, void *arg1, void *arg2, pthread_t id)
{
    ret_val_if_fail(m_worker_thread_pool != NULL, -1);

    return m_worker_thread_pool->post_message(handler, arg1, arg2, id);
}


sint32 Reactor::get_wthread_number()
{
    return m_worker_thread_pool->get_wthread_number();
}

const vector<pthread_t> &Reactor::get_thread_ids()
{
    return m_worker_thread_pool->get_thread_ids();
}


sint32 Reactor::get_thread_pos(pthread_t id)
{
    return m_worker_thread_pool->get_thread_pos(id);
}

}

