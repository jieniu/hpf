/*
 * filename      : connection_info.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 14:39
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _CONNECTION_INFO_H_
#define _CONNECTION_INFO_H_
#include "base_header.h"
#include "job.h"
#include "event_message.h"

namespace hpf
{

class Receiver;
class Sender;
class MessageProcessor;
class AsynOperation;
class MessageHandler;
class ReconnectHandler;

enum ConnectionState
{
    CONN_INIT,
    CONN_CONNECT,
    CONN_CLOSING,
    CONN_CLOSE
};

class ConnectionInfo : public Job
{
    public:
        const static sint32 DEFAULT_CONNECT_TIMEOUT = 10;

        ConnectionInfo(sint32 fd, const char *ip, uint16_t port, 
                MessageProcessor *msg_proc, sint32 is_reconnect, sint32 connect_timeout);

        virtual ~ConnectionInfo();

        virtual sint32 run(sint32 fd, sint16 which);

        virtual sint32 write_data(sint32 no, AsynOperation *ao) = 0;

        virtual sint32 reconnect(uint32 handle) = 0;

        virtual sint32 update_event() = 0;

        virtual sint32 deliver_message(MessageHandler *handler, void *arg1, void*arg2) = 0;

        virtual sint32 close() = 0;

        virtual void delete_event_success();
        
        virtual sint32 set_is_connect();
        
        sint32 add_reconnect_timeout();
        
        sint32 is_connect() {return m_status == CONN_CONNECT;}

        sint32 is_reconnect() {return m_is_reconnect;}

        sint32 get_handle() {return m_handle;}
    
        void get_peer_addr(string &ip, uint16 &port);

        void erase_connection();
    protected:
        virtual sint32 handle_close() = 0;

    protected:
        string m_connected_ip;      // 对端ip
        uint16 m_connected_port;    // 对端port
        Receiver *m_receiver;       // 主要用来接收数据
        Sender *m_sender;           // 发送数据
        MessageProcessor *m_message_processor;
        sint32 m_is_reconnect;      // 是否重连, 只对主动连接有效; 0=no, 1=yes

        sint32 m_handle;            // 连接句柄，使用者用这个句柄向对端发送数据
        uint32 m_reconnect_timer;   // 重连超时句柄
        static ReconnectHandler s_reconnect_handler;

        pthread_mutex_t m_connection_lock;
        ConnectionState m_status;
    protected:
        class CloseHandler : public MessageHandler
        {
        public:
            CloseHandler() {}
            virtual ~CloseHandler() {}
            virtual void process_message(sint32 no, void *arg1, void *arg2)
            {
                ConnectionInfo *ci = reinterpret_cast<ConnectionInfo*>(arg1);
                ci->handle_close();
            }
        };
        static CloseHandler s_close_handler;
};

}
#endif
