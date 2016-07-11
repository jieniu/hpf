#include "receiver.h"
#include "bytebuffer/bytebuffer.h"
#include "base_define.h"
#include <sys/types.h>
#include <sys/socket.h>

namespace hpf
{

/*
 * Function Descript : 读socket
 * Create Time       : 2011-08-24 00:34
 * Parameter List    : 
 *  1. fd: socket fd
 * Return            : 
 *  1. >0: 返回读取数据大小
 *  2. -1: 出错断开
 *  3. 0: 读buffer满或无数据可读
 * Modify Time       : 
 */
sint32 Receiver::read_data(sint32 fd)
{
    sint32 ret = 0;
    // allocate recv buffer
    if (NULL == m_recv_buffer.address())        
    {
        m_recv_length = DEFAULT_RECV_LENGTH;
        char *buf = (char*)malloc(m_recv_length);
        ret_val_if_fail(buf != NULL, -1);

        new (&m_recv_buffer) ByteBuffer(buf, 0, m_recv_length);
    }

    m_recv_buffer.compact();
    
    sint32 read_times = 0;
    while (1)
    {
        if (0 == m_recv_buffer.remaining())
        {
            ret_val_if_fail(m_recv_length <= MAX_RECV_LENGTH, ret);

            if (read_times >= MAX_MALLOC_ONCE)
            {
                return ret;
            }

            sint32 pos = m_recv_length;
            m_recv_length = 2 * m_recv_length;
            char *buf = m_recv_buffer.address();
            buf = (char*)realloc(buf, m_recv_length);
            ret_val_if_fail(buf != NULL, -1);
            
            new (&m_recv_buffer)ByteBuffer(-1, pos, m_recv_length, m_recv_length, buf, 0);
            read_times++;
        }

        // read
        sint32 recv_ret = recv(fd, m_recv_buffer.address() + m_recv_buffer.position(), 
                    m_recv_buffer.remaining(), 0);
        if (recv_ret < 0)
        {
            if (EAGAIN == errno || EINTR == errno)
            {
                return ret;
            }
            // TODO log
            return -1;
        }
        else if (0 == recv_ret)
        {
            return -1;
        }

        ret += recv_ret;

        m_recv_buffer.position(recv_ret + m_recv_buffer.position());
        // TODO test if not return
        if (m_recv_buffer.remaining() > 0)
        {
            return ret;
        }
    }

    return ret;
}


void Receiver::clean()
{
    char *buffer = m_recv_buffer.address();
    if (buffer)
    {
        free(buffer);
        new (&m_recv_buffer)(ByteBuffer)();
        assert(m_recv_buffer.address() == 0);
    }
}

}
