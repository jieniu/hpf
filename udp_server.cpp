#include "udp_server.h"
#include "worker_thread.h"
#include "worker_thread_pool.h"
#include "udp_job.h"
#include "SDSocketUtility.h"
#include "udp_asyn_operation.h"

namespace hpf
{

UdpServer::UdpServer(const char *ip, sint32 port)
    : m_fd(-1), m_ip(ip), m_port(port), m_job_num(0), m_index(0)
{
}

UdpServer::~UdpServer()
{
    for (sint32 i = 0; i < m_job_num; i++)
    {
        m_jobs[i]->get_worker_thread()->delete_job(m_jobs[i]);
    }
    delete [] m_jobs;
    close(m_fd);
    m_fd = -1;
}

sint32 UdpServer::init(WorkerThreadPool *threadpool, MessageProcessor *proc)
{
    sockaddr_in addr;
    m_fd = SDSocketUtility::bind_to(m_ip.c_str(), m_port, &addr);
    if (m_fd == -1)
    {
        LOGGER_ERROR(g_framework_logger, "create udp socket error,ip:" << m_ip 
                     << ",port:" << m_port);
        return -1;
    }
    sint32 thread_num = threadpool->get_wthread_number();
    m_jobs = new UdpJob*[thread_num];
    for (sint32 i = 0; i < thread_num; i++)
    {
        m_jobs[i] = new UdpJob(m_fd, this, proc);
        m_jobs[i]->set_event_flags(EV_READ | EV_PERSIST);
        threadpool->dispatch_new_job(i, m_jobs[i]);
    }
    m_job_num = thread_num;
    return 0;
}

sint32 UdpServer::send_data(UdpAsynOperation *oper)
{
    ByteBuffer &bb = oper->get_bytebuffer();
    socklen_t len = 0;
    sockaddr *addr = oper->get_peer_addr(&len);
    ssize_t ret = sendto(m_fd, bb.address() + bb.position(),
            bb.remaining(), 0, addr, len);
    if (ret == -1)
    {
        LOGGER_ERROR(g_framework_logger, "send udp msg error:" << errno);
        sint32 pos = m_index++ % m_job_num;
        return m_jobs[pos]->push_asyn_operation(oper);
    }
    if (ret != bb.remaining())
    {
        LOGGER_ERROR(g_framework_logger, "udp msg truncate,total len:" 
                << bb.remaining() << ",sent:" << ret);
    }
    oper->on_operation_complete(0, 0);
    return 0; 
}

}

