#include "connection_info.h"
#include "reactor.h"
#include "connection_manager.h"
#include "message_processor.h"
#include "global_var.h"
#include "receiver.h"
#include "sender.h"
#include "timeout_job.h"

namespace hpf
{

class ReconnectHandler : public TimeoutHandler
{
    public:
        ReconnectHandler(){}
        virtual ~ReconnectHandler(){}

        virtual bool handle_timeout(uint32 handle, void *arg)
        {
            ConnectionInfo *ci = reinterpret_cast<ConnectionInfo*>(arg);
            ci->reconnect(handle);
            return true;
        }
};

ReconnectHandler ConnectionInfo::s_reconnect_handler;
ConnectionInfo::CloseHandler ConnectionInfo::s_close_handler;

ConnectionInfo::ConnectionInfo(sint32 fd, const char *ip, uint16_t port, 
        MessageProcessor *msg_proc, sint32 is_reconnect, sint32 connect_timeout)
    : Job(fd)
{
    m_connected_ip = ip;
    m_connected_port = port;

    m_is_reconnect = is_reconnect;

    m_timeval.tv_sec = connect_timeout; // 客户端超时时间
    m_timeval.tv_usec = 0;

    m_handle = ConnectionManager::get_instance()->get_handle(this); 
    m_reconnect_timer = 0;

    m_message_processor = msg_proc;
    m_receiver = NULL;
    m_sender = NULL;
    pthread_mutex_init(&m_connection_lock, NULL);
    m_status = CONN_INIT;
}

ConnectionInfo::~ConnectionInfo()
{
    m_message_processor = NULL;
    m_handle = 0;
    m_receiver = NULL;
    m_sender = NULL;
    pthread_mutex_destroy(&m_connection_lock);
}


sint32 ConnectionInfo::add_reconnect_timeout()
{
    // 正在重连中
    if (m_reconnect_timer != 0)
    {
        LOGGER_DEBUG(g_framework_logger, "reconnecting...");
        return 0;
    }

    timeval tv;
    tv.tv_sec = DEFAULT_CONNECT_TIMEOUT;
    tv.tv_usec = 0;
    if (-1 == Reactor::get_instance()->add_timer(&s_reconnect_handler, 
                this, tv, 0, m_reconnect_timer))
    {
        LOGGER_WARN(g_framework_logger, "reconnect add_timer failed.");
        return -1;
    }

    return 0;
}

void ConnectionInfo::delete_event_success()
{
    if (!m_is_reconnect)
    {
        delete this;
    }
}

sint32 ConnectionInfo::set_is_connect()
{
    sint32 res = 0;
    m_status = CONN_CONNECT;
    if (m_message_processor)
    {
        LOGGER_DEBUG(g_framework_logger, "set is connect.");
        res = m_message_processor->connect(m_handle, m_connected_ip, m_connected_port);
    }
    return res;
}

sint32 ConnectionInfo::run(sint32 fd, sint16 which)
{
    if ((which & EV_TIMEOUT) == EV_TIMEOUT)
    {
        LOGGER_DEBUG(g_framework_logger, "connection timeout." << m_fd);
        if (NULL != m_message_processor)
        {
            m_message_processor->handle_timeout(m_handle);
        }
        return close();
    }

    if (CONN_INIT == m_status)
    {
        if ((EV_READ | EV_WRITE) == (which & (EV_READ | EV_WRITE)))
        {
            // error happend
            return close();
        }
        else if (which & EV_WRITE)
        {
            if (set_is_connect() < 0)
            {
                return close();
            }
            update_event();
        }
        return 0;
    }

    // read event
    sint32 status;
    if (which & EV_READ)
    {
        LOGGER_TRACE(g_framework_logger, "process recv:" << fd);
        status = m_receiver->read_data(fd);
        if (-1 == status)
        {
            return close(); 
        }
//        m_worker_thread->_stat._recv_bytes += status;

        status = m_receiver->process_recv(m_handle);
        if (-1 == status)
        {
            return close();
        }
    }

    // write event
    if (which & EV_WRITE)
    {
        sint32 flush_len = 0;
        status = m_sender->flush_data(m_fd, flush_len);

        if (status < 0)
        {
            return close();
        }
//        _worker_thread->do_update_job(this);
//        _worker_thread->_stat._send_bytes += flush_len;
    }

    if (CONN_CONNECT == m_status)
    {
        update_event();
    }
    return 0;
}

void ConnectionInfo::get_peer_addr(string &ip, uint16 &port)
{
    ip = m_connected_ip;
    port = m_connected_port;
}

void ConnectionInfo::erase_connection()
{
    ConnectionManager::get_instance()->erase_connection(m_handle);
}

}
