#include "htp_connection_info.h"
#include "message_processor.h"
#include "htp_receiver.h"
#include "htp_sender.h"
#include "global_var.h"
#include "SDSocketUtility.h"
#include "reactor.h"
#include "base_define.h"
#include "asyn_operation.h"

namespace hpf
{

Fifo<HtpConnectionInfo*> HtpConnectionInfo::s_free_list;

HtpConnectionInfo::HtpConnectionInfo(sint32 fd, const char *ip, uint16_t port,
                MessageParser *parser, MessageProcessor *msg_proc,
                sint32 is_reconnect, sint32 connect_timeout)
    : ConnectionInfo(fd, ip, port, msg_proc, is_reconnect, connect_timeout)
{
    m_parser = parser;
    m_active_number = 0;
    m_sender_job = NULL;
    m_receiver_job = NULL;

    pthread_mutex_init(&m_init_lock, NULL);
    pthread_cond_init(&m_init_cond, NULL);
}

HtpConnectionInfo::~HtpConnectionInfo()
{
    /* erase_connection 在这里的原因：
     *     防止ConnectionInfo的派生类析构后, ConnectionInfo析构前(共同需要锁m_mutex)
     *     其他线程调用ConnectionManager::close_connection(),
     *     如果出现这种情况就会导致在ConnectionManager找到ConnectionInfon的派生类，
     *     但是其实已经析构掉了. 这样就会出现 __cxa_pure_virtual, 调用纯虚函数的问题
     * 每个ConnectionInfo的派生类都需要调用!
     */
    erase_connection();

    pthread_mutex_destroy(&m_init_lock);
    pthread_cond_destroy(&m_init_cond);
    if (NULL != m_sender_job)
    {
        delete m_sender_job;
        m_sender_job = NULL;
    }
    if (NULL != m_receiver_job)
    {
        delete m_receiver_job;
        m_receiver_job = NULL;
    }
}

sint32 HtpConnectionInfo::write_data(sint32 no, AsynOperation *ao)
{
    sint32 ret = -2;
    if (CONN_CONNECT == m_status)
    {
        ret = ((HtpSender*)m_sender)->push_asyn_operation(no, ao);
    }
    if (ret < 0)
    {
        ao->on_operation_complete(ret, 0);
    }
    return ret;
}

// handle = 超时句柄
sint32 HtpConnectionInfo::reconnect(uint32 handle)
{
    assert(handle == m_reconnect_timer);
    m_reconnect_timer = 0;

    if (m_active_number != 0)
    {
        LOGGER_ERROR(g_framework_logger, "connection reconnect with active thread: "
                     << m_active_number << " interface:" << m_connected_ip
                     << ":" << m_connected_port);
        return add_reconnect_timeout();
    }

    if (!m_is_reconnect)
    {
        delete this; //此对象不会再被引用
        return 0;
    }

    LOGGER_INFO(g_framework_logger, "reconnect to " << m_connected_ip << ":" << m_connected_port);

    m_status = CONN_INIT;
    struct sockaddr_in si;
    m_fd = SDSocketUtility::nonblock_connect_to(m_connected_ip.c_str(), m_connected_port, &si);
    if (-1 == m_fd)
    {
        LOGGER_DEBUG(g_framework_logger, "connect failed and reconnect later: " << m_connected_ip
                     << ":" << m_connected_port);
        return add_reconnect_timeout();
    }
//    ((HtpSender*)m_sender)->set_fd(m_fd);
//    m_sender_job.set_fd(m_fd);
//    m_receiver_job.set_fd(m_fd);

    if (m_active_number != 2)
    {
        init(false);
    }

//    update_event();

    return 0;
}

sint32 HtpConnectionInfo::handle_close()
{
    LOGGER_DEBUG(g_framework_logger, "connection close:" << m_fd << ",connect:"
                 << m_status);
    if (-1 == m_fd)
    {
        return 0;
    }
    if (NULL != m_message_processor && CONN_CLOSING == m_status)
    {
        m_message_processor->disconnect(m_handle);
    }
    m_status = CONN_CLOSE;
    ::close(m_fd);
    m_fd = -1;
    //设置标志后延迟1秒以免其它线程在m_receiver、m_sender销毁后
    //通过write_data、close接口再次访问它们
    sleep(1);
    m_receiver->clean();
    m_sender->clean();

    //延迟一段时间重连或删除对象，以等待m_receiver、m_sender销毁和
    //此连接的命令处理完
    add_reconnect_timeout();
    return 0;
}

/*
 * Function Descript : 初始化并启动receiver线程和sender线程
 * Create Time       : 2011-08-27 00:26
 * Parameter List    :
 *
 * Return            :
 * Modify Time       :
 */
sint32 HtpConnectionInfo::init(bool is_connect)
{
    sint32 timeout = 0;
    timeval *tm = get_timeval();
    if (NULL != tm)
    {
        timeout = tm->tv_sec;
    }
    if (NULL != m_sender_job)
    {
        delete m_sender_job;
    }
    m_sender_job = new HtpJob(this, m_fd, timeout);
    assert(NULL != m_sender_job);
    if (NULL != m_receiver_job)
    {
        delete m_receiver_job;
    }
    m_receiver_job = new HtpJob(this, m_fd, timeout);
    assert(NULL != m_receiver_job);
    m_receiver = new HtpReceiver(this, m_parser, m_message_processor, m_receiver_job);
    assert(m_receiver);
    m_sender = new HtpSender(m_fd, Reactor::get_instance()->get_wthread_number(), this, m_sender_job);
    assert(m_sender);

    ret_val_if_fail(0 == ((HtpReceiver*)m_receiver)->init(), -1);
    ret_val_if_fail(0 == ((HtpSender*)m_sender)->init(), -1);

    if (!is_connect)
    {
        m_receiver_job->set_event_flags(EV_READ | EV_WRITE);
        ((HtpReceiver*)m_receiver)->do_add_job(m_receiver_job);
    }

    ret_val_if_fail(0 == ((HtpSender*)m_sender)->start_thread(), -1);
    ret_val_if_fail(0 == ((HtpReceiver*)m_receiver)->start_thread(), -1);

    pthread_mutex_lock(&m_init_lock);
    while (m_active_number < 2)
    {
        pthread_cond_wait(&m_init_cond, &m_init_lock);
    }
    pthread_mutex_unlock(&m_init_lock);

    LOGGER_INFO(g_framework_logger, "init htp_connection_info success, dest ip = "
                << m_connected_ip << ":" << m_connected_port);

    return 0;
}

sint32 HtpConnectionInfo::set_is_connect()
{
    sint32 ret = 0;
    if (CONN_CONNECT != m_status)
    {
        ret = ConnectionInfo::set_is_connect();
        if (0 == ret)
        {
            m_receiver_job->set_event_flags(EV_READ | EV_PERSIST);
            ret = ((HtpReceiver*)m_receiver)->add_new_job(m_receiver_job);
        }
    }
    return ret;
}

void HtpConnectionInfo::count_active_thread()
{
    pthread_mutex_lock(&m_init_lock);
    m_active_number++;
    pthread_cond_signal(&m_init_cond);
    pthread_mutex_unlock(&m_init_lock);
}

void HtpConnectionInfo::decrease_active_thread()
{
    pthread_mutex_lock(&m_init_lock);
    m_active_number--;
    pthread_mutex_unlock(&m_init_lock);
}

sint32 HtpConnectionInfo::update_event()
{
    if (CONN_CONNECT != m_status)
    {
        return -1;
    }
    if (m_sender->get_ao_count() > 0)
    {
        m_sender_job->set_event_flags(EV_WRITE);
        return ((HtpSender*)m_sender)->update_job(m_sender_job);
    }
//    else if (0 == m_is_connect)
//    {
//        m_sender_job.set_event_flags(EV_READ | EV_WRITE);
//        return ((HtpSender*)m_sender)->add_new_job(&m_sender_job);
//    }
    return 0;
}

sint32 HtpConnectionInfo::deliver_message(MessageHandler *handler, void *arg1, void*arg2)
{
    return Reactor::get_instance()->post_message(handler, arg1, arg2);
}

sint32 HtpConnectionInfo::close()
{
    ConnectionState old_status;
    pthread_mutex_lock(&m_connection_lock);
    old_status = m_status;
    if (CONN_INIT == m_status)
    {
        m_status = CONN_CLOSE;
    }
    else if (CONN_CONNECT == m_status)
    {
        m_status = CONN_CLOSING;
    }
    pthread_mutex_unlock(&m_connection_lock);
    if (CONN_CLOSE == old_status || CONN_CLOSING == old_status)
    {
        return 0;
    }
    sint32 ret = ((HtpReceiver*)m_receiver)->post_message(&s_close_handler,
            reinterpret_cast<void*>(static_cast<ConnectionInfo*>(this)), NULL);
    if (ret < 0)
    {
        pthread_mutex_lock(&m_connection_lock);
        m_status = old_status;
        pthread_mutex_unlock(&m_connection_lock);
    }
    return ret;
}

void* HtpConnectionInfo::operator new(size_t len)
{
    HtpConnectionInfo *p = NULL;
    s_free_list.pop(p);
    if (NULL == p)
    {
        LOGGER_TRACE(g_framework_logger, "alloc new HtpConnectionInfo");
        return malloc(len);
    }
    return (void*)p;
}

void HtpConnectionInfo::operator delete(void* p, size_t len)
{
    s_free_list.push_back((HtpConnectionInfo*)p);
}

void HtpConnectionInfo::process_transmissive_message(ByteBuffer *bb)
{
    if (NULL != m_message_processor)
    {
        m_message_processor->process_input(bb, get_handle());
    }
}

}
