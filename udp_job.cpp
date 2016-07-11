#include "udp_job.h"
#include "message_processor.h"
#include "udp_asyn_operation.h"
#include "worker_thread.h"

namespace hpf
{

UdpJob::UdpJob (sint32 fd, UdpServer *udpserver, MessageProcessor *processor)
    : Job(fd), m_udp_server(udpserver), m_processor(processor)
{
    char *buff = new char[MAX_PACKAGE_LEN];
    assert(buff != NULL);
    new (&m_recv_buff) ByteBuffer(buff, 0, MAX_PACKAGE_LEN);
}

UdpJob::~UdpJob()
{
    m_udp_server = NULL;
    m_processor = NULL;
    clear_sendbuf_list();
    char *buff = m_recv_buff.address();
    new (&m_recv_buff) ByteBuffer();
    delete [] buff;
}

void UdpJob::clear_sendbuf_list()
{
    UdpAsynOperation *oper;
    while (m_sendbuf_list.pop(oper) == 0)
    {
        oper->on_operation_complete(-2, 0);
    }
}

sint32 UdpJob::run(sint32 fd, sint16 which)
{
    assert(fd == m_fd);
    if (which & EV_READ)
    {
        LOGGER_TRACE(g_framework_logger, "process recv:" << fd);
        process_recv();
    }

    if (which & EV_WRITE)
    {
        LOGGER_TRACE(g_framework_logger, "process write:" << fd);
        process_send();
        update_event();
    }
    return 0;
}

void UdpJob::delete_event_success()
{
    delete this;
}

sint32 UdpJob::process_recv()
{
    m_recv_buff.position(0);
    m_recv_buff.compact();
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ssize_t ret = recvfrom(m_fd, m_recv_buff.address() + m_recv_buff.position(),
                    m_recv_buff.remaining(), 0, (sockaddr*)&addr, &addrlen);
    if (ret == -1)
    {
        LOGGER_WARN(g_framework_logger, "recv udp message errro,fd:" << m_fd);
        return -1;
    }
    if (ret == 0)
    {
        return 0;
    }
    m_recv_buff.position(ret);
    m_recv_buff.flip();

    if (m_processor != NULL)
    {
        return m_processor->process_udp_message(&m_recv_buff,
                    m_udp_server, (sockaddr*)&addr, addrlen);
    }
    return 0;
}

sint32 UdpJob::process_send()
{
    UdpAsynOperation *oper;
    while (m_sendbuf_list.pop(oper) == 0)
    {
        ByteBuffer &bb = oper->get_bytebuffer();
        socklen_t len = 0;
        sockaddr *addr = oper->get_peer_addr(&len);
        ssize_t ret = sendto(m_fd, bb.address() + bb.position(),
                             bb.remaining(), 0, addr, len);
        if (ret == -1)
        {
            LOGGER_ERROR(g_framework_logger, "send udp msg error:" << errno);
            m_sendbuf_list.push_front(oper);
            break;
        }
        if (ret != bb.remaining())
        {
            LOGGER_ERROR(g_framework_logger, "udp msg truncate,total len:" 
                         << bb.remaining() << ",sent:" << ret);
        }
        oper->on_operation_complete(0, 0);
    }
    return 0;
}

sint32 UdpJob::update_event()
{
    if (m_sendbuf_list.size() > 0)
    {
        set_event_flags(EV_READ | EV_WRITE);
    }
    else
    {
        set_event_flags(EV_READ | EV_PERSIST);
    }
    return m_worker_thread->do_update_job(this);
}

sint32 UdpJob::push_asyn_operation(UdpAsynOperation *oper)
{
    m_sendbuf_list.push_back(oper);
    set_event_flags(EV_READ | EV_WRITE);
    return m_worker_thread->update_job(this);
}

}

