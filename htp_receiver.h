/*
 * filename      : htp_receiver.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 11:26
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_RECEIVER_H_
#define _HTP_RECEIVER_H_

#include "receiver.h"
#include "event_thread.h"

namespace hpf
{

class MessageProcessor;
class HtpConnectionInfo;
class HtpReceiverCleanHandler;
class MessageParser;
class HtpMessageDeliverer;
class HtpJob;

class HtpReceiver : public Receiver, public EventThread
{
    public:
        HtpReceiver(HtpConnectionInfo *ci, MessageParser *parser, 
                    MessageProcessor *mp, HtpJob *job)
            : Receiver(mp), m_htp_connection_info(ci), m_parser(parser), m_job(job)
        {}
        virtual ~HtpReceiver();
        
        virtual sint32 process_recv(sint32 handle);

        // 线程初始化时调用
        virtual sint32 self_init();

        // 线程进入事件循环前调用
        virtual sint32 begin_routine();

        // 线程停止事件循环前调用
        // 最好在这里删除掉所有的事件, 否则可能会崩溃
        virtual sint32 end_routine();

        virtual void clean();

        void do_clean();

    private:
        HtpConnectionInfo *m_htp_connection_info;
        MessageParser *m_parser;
        HtpJob *m_job;

        static HtpReceiverCleanHandler s_htp_receiver_clean_handler;
        static HtpMessageDeliverer s_htp_message_deliverer;
};

}
#endif
