#include "hcc_listen_thread.h"
#include "hcc_connection_info.h"
#include "worker_thread_pool.h"
#include "SDSocketUtility.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace hpf
{

sint32 HccListenThread::add_new_connect(sint32 fd)
{
    // accept
    sockaddr_in addr;
    sint32 newfd = SDSocketUtility::accept_client(fd, &addr);    
    if (-1 == newfd)
    {
        LOGGER_ERROR(g_framework_logger, "hcc accept client failed.");
        return 0;
    }

    SDSocketUtility::set_linger(newfd, 1, 0);

    // add a connect timeout event when accept a client
    char *ip = inet_ntoa(addr.sin_addr);
    SDSocketUtility::set_nonblock(newfd);
    uint16 port = ntohs(addr.sin_port);
    HccConnectionInfo *ci = new HccConnectionInfo(newfd, ip, port, 
            m_message_processor, false, m_connect_timeout);
    LOGGER_DEBUG(g_framework_logger, "accept a new client, " << ip << ":" << port);
    assert(ci);
    if (ci->set_is_connect() < 0)
    {
        delete ci;
        return 0;
    }

    ci->set_event_flags(EV_READ);
    // send new connection to worker thread
    if(-1 == m_thread_pool->dispatch_new_job(ci))
    {
        // added by fengyajie
        delete ci;
    }
    return 0;
}

}
