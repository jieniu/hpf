/*
 * filename      : event_handler.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-15 20:52
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_
#include "job.h"

namespace hpf
{

class EventHandler
{
    public:
        EventHandler();
        virtual ~EventHandler();

        static void handler(sint32 fd, short which, void* arg);
};

inline 
void EventHandler::handler(sint32 fd, short which , void* arg)
{
    Job *job = (Job*)arg;
    job->run(fd, which);
}

}
#endif
