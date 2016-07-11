#include "hcc_connection_info.h"
#include "message_processor.h"
#include "worker_thread.h"
#include "SDSocketUtility.h"
#include "connection_manager.h"
#include "asyn_operation.h"
#include "hcc_receiver.h"
#include "hcc_sender.h"

namespace hpf
{

Fifo<HccConnectionInfo*> HccConnectionInfo::s_free_list;

HccConnectionInfo::HccConnectionInfo(sint32 fd, const char *ip, uint16_t port,
        MessageProcessor *msg_proc, sint32 is_reconnect, sint32 connect_timeout)
    : ConnectionInfo(fd, ip, port, msg_proc, is_reconnect, connect_timeout)
{
    m_receiver = new HccReceiver(msg_proc);
    m_sender = new HccSender();

    assert(m_receiver);
    assert(m_sender);
}

HccConnectionInfo::~HccConnectionInfo()
{
    /* erase_connection 在这里的原因：
     *     防止ConnectionInfo的派生类析构后, ConnectionInfo析构前(共同需要锁m_mutex)
     *     其他线程调用ConnectionManager::close_connection(),
     *     如果出现这种情况就会导致在ConnectionManager找到ConnectionInfon的派生类，
     *     但是其实已经析构掉了. 这样就会出现 __cxa_pure_virtual, 调用纯虚函数的问题
     * 每个ConnectionInfo的派生类都需要调用!
     */
    erase_connection();

    delete m_receiver;
    delete m_sender;
    m_receiver = NULL;
    m_sender = NULL;
}

sint32 HccConnectionInfo::write_data(sint32 no, AsynOperation *ao)
{
    sint32 write_len = 0;
    sint32 ret = m_sender->write_data(m_fd, ao, write_len);
    if (-1 == ret)
    {
        return close();
    }
    else if (0 == ret)
    {
        m_event_flags = EV_WRITE | EV_READ;
    }
    else
    {
        m_event_flags = EV_READ;
    }

    m_worker_thread->do_update_job(this);

    return ret;
}

sint32 HccConnectionInfo::reconnect(uint32 handle)
{
    assert(handle == m_reconnect_timer);
    m_reconnect_timer = 0;

    struct sockaddr_in addr;
    LOGGER_DEBUG(g_framework_logger, "reconnect to " << m_connected_ip << ":" << m_connected_port);
    m_fd = SDSocketUtility::nonblock_connect_to(m_connected_ip.c_str(), m_connected_port, &addr);
    if (-1 == m_fd)
    {
        LOGGER_DEBUG(g_framework_logger, "connect failed and reconnect later: " << m_connected_ip
                     << ":" << m_connected_port);
        add_reconnect_timeout();
    }
    m_status = CONN_INIT;
    m_event_flags = EV_READ | EV_WRITE;
    m_worker_thread->add_new_job(this);

    return 0;
}

sint32 HccConnectionInfo::handle_close()
{
    LOGGER_DEBUG(g_framework_logger, "connection close:" << m_fd << ",status:"
                 << m_status);
    if (-1 == m_fd)
    {
        return 0;
    }
    if (m_message_processor && CONN_CLOSING == m_status)
    {
        m_message_processor->disconnect(m_handle);
    }
    m_status = CONN_CLOSE;

    ::close(m_fd);
    m_fd = -1;
    sint32 is_reconnect = m_is_reconnect;

    m_receiver->clean();
    m_sender->clean();

    if (is_reconnect)
    {
        LOGGER_DEBUG(g_framework_logger, "connect closed, reconnect later: " << m_connected_ip
                << ":" << m_connected_port);
        add_reconnect_timeout();
    }

    // 如果不重连，connection info在移除event_base后会被delete
    m_worker_thread->do_delete_job(this);
    return 0;
}

sint32 HccConnectionInfo::update_event()
{
    if (CONN_CONNECT != m_status)
    {
        return -1;
    }
    if (m_sender->get_ao_count() > 0)
    {
        m_event_flags = EV_READ | EV_WRITE;
    }
    else
    {
        m_event_flags = EV_READ;
    }

    m_worker_thread->do_update_job(this);

    return 0;
}

sint32 HccConnectionInfo::deliver_message(MessageHandler *handler, void *arg1, void*arg2)
{
    return m_worker_thread->post_message(handler, arg1, arg2);
}

sint32 HccConnectionInfo::close()
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
    sint32 ret = deliver_message(&s_close_handler,
            reinterpret_cast<void*>(static_cast<ConnectionInfo*>(this)), NULL);
    if (ret < 0)
    {
        pthread_mutex_lock(&m_connection_lock);
        m_status = old_status;
        pthread_mutex_unlock(&m_connection_lock);
    }
    return ret;
}

void* HccConnectionInfo::operator new(size_t len)
{
    HccConnectionInfo *p = NULL;
    s_free_list.pop(p);
    if (NULL == p)
    {
        LOGGER_TRACE(g_framework_logger, "alloc new HccConnectionInfo");
        return malloc(len);
    }
    return (void*)p;
}

void HccConnectionInfo::operator delete(void* p, size_t len)
{
    s_free_list.push_back((HccConnectionInfo*)p);
}

}
