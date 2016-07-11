#include "htp_receiver.h"
#include "job.h"
#include "reactor.h"
#include "htp_connection_info.h"
#include "event_message.h"  
#include "message_parser.h"
#include "message_processor.h"
#include "recycle_thread.h"
#include "htp_job.h"

namespace hpf
{

class HtpReceiverCleanHandler : public MessageHandler
{
    public:
        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            HtpReceiver *htp_receiver = (HtpReceiver*)arg1;
            htp_receiver->do_clean();            
        }
};

class HtpMessageDeliverer : public MessageHandler
{
    public:
        HtpMessageDeliverer(){}
        virtual ~HtpMessageDeliverer(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            HtpConnectionInfo *conn = static_cast<HtpConnectionInfo*>(arg1); 
            conn->process_transmissive_message((ByteBuffer*)arg2);
        }
};

HtpReceiverCleanHandler HtpReceiver::s_htp_receiver_clean_handler;
HtpMessageDeliverer HtpReceiver::s_htp_message_deliverer;

HtpReceiver::~HtpReceiver()
{
    m_htp_connection_info = NULL;
    m_parser = NULL;
    m_job = NULL;
}

sint32 HtpReceiver::process_recv(sint32 handle)
{
    m_recv_buffer.flip();

    // user need move the buffer curser
    // user should new the ByteBuffer himself
    Fifo<ByteBuffer*> packets;
    if (-1 == m_parser->parse_message(m_recv_buffer, packets))
    {
        return -1;
    }

    ByteBuffer *bb;
    while (packets.pop(bb) != -1)
    {
        Reactor::get_instance()->post_message(&s_htp_message_deliverer, 
                                              m_htp_connection_info, bb);
    }

    return 0;
}

// 线程初始化时调用
sint32 HtpReceiver::self_init()
{
    return 0;
}

// 线程进入事件循环前调用
sint32 HtpReceiver::begin_routine()
{
    m_htp_connection_info->count_active_thread();
    return 0;
}

// 线程停止事件循环前调用
// 最好在这里删除掉所有的事件, 否则可能会崩溃
sint32 HtpReceiver::end_routine()
{
    m_htp_connection_info->decrease_active_thread();
    return 0;
}

void HtpReceiver::clean()
{
    post_message(&s_htp_receiver_clean_handler, this, NULL);
}

void HtpReceiver::do_clean()
{
    do_delete_job(m_job);
    m_job->set_fd(-1);
    Receiver::clean();
    RecycleThread::get_instance()->recycle_thread(this);
}

}
