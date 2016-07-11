/*
 * filename      : udp_job.h
 * descriptor    :  
 * author        : chenyuhui
 * create time   : 2012-07-03 14:06
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date           | who           | modify summary            |
 * +----------------+---------------+---------------------------+
 */
#ifndef _UDP_JOB_H_
#define _UDP_JOB_H_

#include "base_header.h"
#include "fifo.h"
#include "job.h"
#include "bytebuffer/bytebuffer.h"

namespace hpf
{

class UdpAsynOperation;
class UdpServer;
class MessageProcessor;

class UdpJob : public Job
{
public:
    UdpJob (sint32 fd, UdpServer *udpserver, MessageProcessor *processor = NULL);
    virtual ~UdpJob();

    virtual sint32 run(sint32 fd, sint16 which);
    virtual void delete_event_success();

    sint32 push_asyn_operation(UdpAsynOperation *oper);
private:
    sint32 process_recv();
    sint32 process_send();
    sint32 update_event();
    void clear_sendbuf_list();
private:
    enum CONST_VAL{ MAX_PACKAGE_LEN = 1024 * 1024 };
    UdpServer *m_udp_server;
    MessageProcessor *m_processor;
    ByteBuffer m_recv_buff;
    typedef Fifo<UdpAsynOperation *> SendBufferList;
    SendBufferList m_sendbuf_list;
};

}

#endif
