/*
 * filename      : event_thread.h
 * descriptor    : 事件线程
 * author        : fengyajie
 * create time   : 2011-08-03 14:06
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _EVENT_THREAD_H_
#define _EVENT_THREAD_H_
#include "base_header.h"
#include "event_base.h"
#include "job.h"
#include "event_message.h"

namespace hpf
{

class AddJobHandler;
class UpdateJobHandler;
class DeleteJobHandler;
class StopThreadHandler;

class EventThread : public Job
{
    public:
        EventThread();
        virtual ~EventThread();

        // 线程初始化时调用
        virtual sint32 self_init() = 0;

        // 线程进入事件循环前调用
        virtual sint32 begin_routine() = 0;

        // 线程停止事件循环前调用
        // 最好在这里删除掉所有的事件, 否则可能会崩溃
        virtual sint32 end_routine() = 0;

        virtual sint32 get_no() {return -1;}

        sint32 init();

        sint32 start_thread();

        static void* thread_routine(void* arg);
        
        virtual sint32 run(sint32 fd, sint16 which);

        sint32 post_message(MessageHandler *handler, void *arg1, void *arg2);

        sint32 send_message(MessageHandler *handler, void *arg1, void *arg2);

        virtual void delete_event_success() {} 

        sint32 add_new_job(Job *job);
        
        sint32 do_add_job(Job *job);

        sint32 update_job(Job *job);
        
        sint32 do_update_job(Job *job);

        sint32 delete_job(Job *job);

        sint32 do_delete_job(Job *job);

        sint32 stop_routine();

        sint32 do_stop_routine();

        sint32 recycle_resource();

    protected:
        EventBase m_event_base;
        pthread_t m_thread_id;

        sint32 m_message_fd; // you can send message to thread via this fd
        MessageQueue m_message_queue;

        static AddJobHandler s_add_job_handler;
        static UpdateJobHandler s_update_job_handler;
        static DeleteJobHandler s_delete_job_handler;
        static StopThreadHandler s_stop_thread_handler;
};

}
#endif
