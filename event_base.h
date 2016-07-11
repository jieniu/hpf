/*
 * filename      : event_base.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-15 22:34
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _EVENT_BASE_H_
#define _EVENT_BASE_H_
#include "base_header.h"
#include <event.h>

namespace hpf
{

class Job;
class EventBase
{
    public:
        EventBase();
        ~EventBase();

        sint32 init_event_base();

        sint32 add_new_event(Job *job);
        
        sint32 update_event(Job *job);

        sint32 delete_event(Job *job);

        sint32 event_loop();

        sint32 event_loop_exit();
        
    private:
        struct event_base *m_event_base;
};

}
#endif
