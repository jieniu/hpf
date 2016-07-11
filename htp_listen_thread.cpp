#include "htp_listen_thread.h"
#include "htp_connection_info.h"
#include "SDSocketUtility.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace hpf
{

sint32 HtpListenThread::add_new_connect(sint32 fd)
{
    // accept
    sockaddr_in addr;
    sint32 newfd = SDSocketUtility::accept_client(fd, &addr);    
    if (-1 == newfd)
    {
        LOGGER_ERROR(g_framework_logger, "htp accept client failed.");
        return 0;
    }

    SDSocketUtility::set_linger(newfd, 1, 0);

    // add a connect timeout event when accept a client
    char *ip = inet_ntoa(addr.sin_addr);
    SDSocketUtility::set_nonblock(newfd);
    HtpConnectionInfo *ci = new HtpConnectionInfo(newfd, ip, addr.sin_port, 
            m_parser, m_message_processor, false, m_connect_timeout);
    assert(ci);
    ci->init(true);
    if (ci->set_is_connect() < 0)
    {
        LOGGER_ERROR(g_framework_logger, "set connect failed, newfd:" << newfd);
        ci->close();
        return -1;
    }

//    LOGGER_DEBUG(g_framework_logger, "create htp_connection_info: addr=" << ci);
//    if (ci->update_event() != 0)
//    {
//        LOGGER_ERROR(g_framework_logger, "add connector failed: add event error.");
//        ci->close();
//        return -1;
//    }
    return 0;
}

}
