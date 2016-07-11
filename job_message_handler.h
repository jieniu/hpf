/*
 * filename      : job_message_handler.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-03 21:52
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _JOB_MESSAGE_HANDLER_H_
#define _JOB_MESSAGE_HANDLER_H_

#include "event_message.h"
#include "event_thread.h"

namespace hpf
{

class AddJobHandler : public MessageHandler
{
    public:
        AddJobHandler(){}
        virtual ~AddJobHandler(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            EventThread *wt = (EventThread*)arg1;
            wt->do_add_job((Job*)arg2);
        }
};

class UpdateJobHandler : public MessageHandler
{
    public:
        UpdateJobHandler(){}
        virtual ~UpdateJobHandler(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            EventThread *wt = (EventThread*)arg1;
            wt->do_update_job((Job*)arg2);
        }
};

class DeleteJobHandler : public MessageHandler
{
    public:
        DeleteJobHandler(){}
        virtual ~DeleteJobHandler(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            EventThread *wt = (EventThread*)arg1;
            wt->do_delete_job((Job*)arg2);
        }
};

}
#endif

