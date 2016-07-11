#include "sender.h"
#include "asyn_operation.h"
#include "bytebuffer/bytebuffer.h"
#include <sys/types.h>
#include <sys/socket.h>

namespace hpf
{

/*
 * Function Descript : 向某个socket发送数据
 * Create Time       : 2011-08-24 09:20
 * Parameter List    : 
 *  1. fd: socket
 *  2. ao: 异步操作
 * Return            : 
 *  1. -1: 连接错误
 *  2. 0: 中断或暂时不能写
 *  3. >0: 发送的字节数
 * Modify Time       : 
 */
sint32 Sender::write_data(sint32 fd, AsynOperation *ao, sint32 &write_len)
{
//    ret_val_if_fail(fd == _fd, -1);
    // push back
    if (0 != m_send_buffer_list.size())
    {
        m_send_buffer_list.push_back(ao); 
        return 0;
    }

    sint32 ret = do_write_data(fd, ao, write_len);
    if (0 == ret)
    {
        m_send_buffer_list.push_back(ao);
    }

    return ret;

}

/*
 * Function Descript : 具体的写操作
 * Create Time       : 2011-08-24 10:32
 * Parameter List    : 
 *  1. 0: 暂时不能写
 *  2. >0: 写出的字节数
 *  3. <0: 写出错
 * Return            : 
 * Modify Time       : 
 */
sint32 Sender::do_write_data(sint32 fd, AsynOperation *ao, sint32 &write_len)
{
//    ret_val_if_fail(fd == _fd, -1);
    // send 
    sint32 ret = 0;
    write_len = 0;
    ByteBuffer &bb = ao->get_bytebuffer();
    while (1)
    {
        ret = send(fd, bb.address() + bb.position(), bb.remaining(), 0); 
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                return 0;
            }

            // TODO LOG ERROR & stat
            ao->on_operation_complete(-1, errno);
            return -1; 
        }

        bb.position(bb.position() + ret);
        write_len += ret;
        if (bb.remaining() <= 0)
        {
            break;
        }
    }

    ao->on_operation_complete(0, 0);

    return write_len;
}
        
/*
 * Function Descript : 输出缓存中所有数据
 * Create Time       : 2011-08-24 11:01
 * Parameter List    : 
 *  1. fd: socket
 *  2. &flush_len: 输出字节数
 * Return            : 
 *  1. 0: 暂时发不出去
 *  2. -1: 发送失败
 *  3. >0: 发出的字节数
 * Modify Time       : 
 */
sint32 Sender::flush_data(sint32 fd, sint32 &flush_len)
{
//    ret_val_if_fail(fd == _fd, -1);
    AsynOperation *ao = NULL;
    flush_len = 0;
    while (-1 != m_send_buffer_list.pop(ao))
    {
        sint32 write_len = 0;
        sint32 ret = do_write_data(fd, ao, write_len);
        if (0 == ret)
        {
            flush_len += write_len;
            m_send_buffer_list.push_front(ao);
            return 0;
        }
        if (ret < 0)
        {
            return -1;
        }
    
        flush_len += write_len;
    }

    return flush_len;
}

}        
