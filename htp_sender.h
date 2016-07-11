/*
 * filename      : htp_sender.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 20:44
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_SENDER_H_
#define _HTP_SENDER_H_
#include "sender.h"
#include "event_thread.h"

namespace hpf
{

class Channel;
class HtpConnectionInfo;
class HtpSenderCleanHandler;
class HtpJob;

class HtpSender : public Sender, public EventThread
{
    public:

        HtpSender(sint32 fd, sint32 max_thread_number, HtpConnectionInfo *ci, HtpJob *job);
        virtual ~HtpSender();

        virtual void clean();

        void do_clean();

        virtual sint32 self_init();

        // 线程进入事件循环前调用
        virtual sint32 begin_routine();

        // 线程停止事件循环前调用
        // 最好在这里删除掉所有的事件, 否则可能会崩溃
        virtual sint32 end_routine();

        // 添加异步写事件
        sint32 push_asyn_operation(sint32 no, AsynOperation *ao);

        // 获取写事件，并向socket写数据
        sint32 pop_asyn_operation(sint32 fd);

        void set_fd(sint32 fd) {m_fd = fd;}
        
    private:
        sint32 m_fd; // socket
        sint32 m_max_thread_number;
        sint32 m_is_reconnect;
        Channel *m_channels; 
        HtpConnectionInfo *m_htp_connection_info;
        HtpJob *m_job;

        static HtpSenderCleanHandler s_htp_sender_clean_handler;
};

}
#endif
