/*
 * filename      : udp_asyn_operation.h
 * descriptor    :  
 * author        : chenyuhui
 * create time   : 2012-07-03 19:57
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date           | who           | modify summary            |
 * +----------------+---------------+---------------------------+
 */
#ifndef _UDP_ASYN_OPERATION_H_
#define _UDP_ASYN_OPERATION_H_

#include "default_asyn_operation.h"
#include <sys/socket.h>
#include <netinet/in.h>

namespace hpf
{

class UdpAsynOperation : public DefaultAsynOperation
{
public:
    UdpAsynOperation(ByteBuffer &bb, sockaddr *addr, socklen_t addr_len)
        : DefaultAsynOperation(bb)
    {
        assert(addr_len <= sizeof(m_peer_addr));
        memcpy((void*)&m_peer_addr, (void*)addr, addr_len);
        m_addr_len = addr_len;
    }
    virtual ~UdpAsynOperation() {}
    sockaddr* get_peer_addr(socklen_t *len)
    {
        *len = m_addr_len;
        return (sockaddr*)&m_peer_addr;
    }
private:
    sockaddr_in m_peer_addr;
    socklen_t m_addr_len;
};

}
#endif
