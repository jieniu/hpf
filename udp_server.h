/*
 * filename      : udp_server.h
 * descriptor    :  
 * author        : chenyuhui
 * create time   : 2012-07-04 15:21
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date           | who           | modify summary            |
 * +----------------+---------------+---------------------------+
 */
#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include "base_header.h"
namespace hpf
{

class WorkerThreadPool;
class MessageProcessor;
class UdpAsynOperation;
class UdpJob;

class UdpServer
{
public:
    UdpServer(const char *ip, sint32 port);
    ~UdpServer();

    sint32 init(WorkerThreadPool *threadpool, MessageProcessor *proc);
    sint32 send_data(UdpAsynOperation *oper);
private:
    sint32 m_fd;
    string m_ip;
    sint32 m_port;

    UdpJob **m_jobs;
    sint32 m_job_num;
    uint32 m_index;
};

}
#endif
