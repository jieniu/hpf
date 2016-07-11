/*
 * filename      : htp_connection_info.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 00:12
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_CONNECTION_INFO_H_
#define _HTP_CONNECTION_INFO_H_
#include "connection_info.h"
#include "base_header.h"
#include "fifo.h"
#include "htp_job.h"

class ByteBuffer;
namespace hpf
{

class HtpReceiver;
class HtpSender;
class MessageProcessor;
class MessageParser;
class AsynOperation;

class HtpConnectionInfo : public ConnectionInfo
{
    public:
        HtpConnectionInfo(sint32 fd, const char *ip, uint16_t port, MessageParser *parser,
                MessageProcessor *msg_proc, sint32 is_reconnect, sint32 connect_timeout);
        virtual ~HtpConnectionInfo();
        
        virtual sint32 write_data(sint32 no, AsynOperation *ao);

        // handle = 超时句柄
        virtual sint32 reconnect(uint32 handle);

        virtual sint32 close();

        virtual sint32 update_event();

        virtual void delete_event_success() {}

        virtual sint32 deliver_message(MessageHandler *handler, void *arg1, void*arg2);

        virtual sint32 set_is_connect();

        sint32 init(bool is_connect);

        void count_active_thread();
        void decrease_active_thread();

        void process_transmissive_message(ByteBuffer *bb);
    public:
        void* operator new(size_t len);
        void operator delete(void* p, size_t len);
    protected:
        //只在接收线程中调用 
        virtual sint32 handle_close();

    private:
        MessageParser *m_parser;
        sint32 m_active_number; // 目前仅有两个
        pthread_mutex_t m_init_lock;
        pthread_cond_t m_init_cond;

        HtpJob *m_sender_job;
        HtpJob *m_receiver_job;
    private:
        static Fifo<HtpConnectionInfo*> s_free_list;
};

}
#endif
