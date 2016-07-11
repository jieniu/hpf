#include "listen_thread.h"
#include "SDSocketUtility.h"
#include "base_define.h"
#include "job.h"

namespace hpf
{

class ListenJob : public Job
{
    public:
        ListenJob(ListenThread *lt)
            : Job(-1), m_listen_thread(lt)
        {}
        virtual ~ListenJob()
        {
            m_listen_thread = NULL;
            if (-1 != m_fd)
            {
                close(m_fd);
            }
            m_fd = -1;
        }

        virtual sint32 run(sint32 fd, sint16 which)
        {
            assert(fd == m_fd);
            if (m_listen_thread) 
            {
                return m_listen_thread->add_new_connect(fd);
            }

            return -1;
        }

        virtual void delete_event_success() {delete this;}
        
    private:
        ListenThread *m_listen_thread;
};

ListenThread::ListenThread(const char *ip, uint16 port, WorkerThreadPool *tp, 
        MessageProcessor *msg_proc, sint32 timeout)
: m_host_ip(ip), m_host_port(port), m_thread_pool(tp), m_message_processor(msg_proc), 
     m_connect_timeout(timeout)
{
}

ListenThread::~ListenThread()
{
}

sint32 ListenThread::self_init()
{
    m_listen_job = new ListenJob(this);
    ret_val_if_fail((m_listen_job > 0), -1);

    // ´´½¨socket
    sockaddr_in saddr;
    sint32 fd = SDSocketUtility::listen_to(m_host_ip.c_str(), (sint32)m_host_port, &saddr, false);
    if (-1 == fd)
    {
        LOGGER_ERROR(g_framework_logger, "create listen socket error: " << strerror(errno));
        return -1;
    }
    m_listen_job->set_fd(fd);

    sint16 event_flags =  EV_READ | EV_PERSIST;
    m_listen_job->set_event_flags(event_flags);
    if (-1 == do_add_job(m_listen_job))
    {
        LOGGER_ERROR(g_framework_logger, "create listen socket error: " << strerror(errno));
        return -1;
    }

    LOGGER_INFO(g_framework_logger, "init listen thread success.");
    return 0;
}


sint32 ListenThread::end_routine() 
{
    do_delete_job(m_listen_job);
    LOGGER_INFO(g_framework_logger, "end listen thread routine success, ip=" << m_host_ip
            << ", port=" << m_host_port);
    return 0;
}

}
