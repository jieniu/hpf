#include "event_base.h"
#include "job.h"
#include "event_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <event.h>
#include "global_var.h"

namespace hpf
{

EventBase::EventBase()
{
}

EventBase::~EventBase()
{
    LOGGER_DEBUG(g_framework_logger, "free event base = " << m_event_base);
    event_base_free(m_event_base);
    m_event_base = NULL;
}

sint32 EventBase::init_event_base()
{
    m_event_base = event_init();
    if (NULL == m_event_base)
    {
        return -1;
    }
    return 0;
}

/*
 * Function Descript : add a new event
 * Create Time       : 2010-12-15 22:36
 * Parameter List    : 
 *  Job *job: 
 * Return            : 
 *  1. 0=success
 *  2. -1=failed
 * Modify Time       : 
 */
sint32 EventBase::add_new_event(Job *job)
{
    if (NULL == job->get_event())
    {
        return -1;
    }

    sint32 fd = job->get_fd() > 0 ? job->get_fd() : -1;
    event_set(job->get_event(), 
              fd, 
              (sint16)job->get_event_flags(),
              EventHandler::handler,
              job);

    event_base_set(m_event_base, job->get_event());
    
    struct timeval *ptv = job->get_timeval();

    if (-1 == event_add(job->get_event(), ptv))
    {
        return -1;
    }

    return 0;
}


/*
 * Function Descript : get into event loop
 * Create Time       : 2010-12-15 22:39
 * Parameter List    : 
 * 
 * Return            : 
 * Modify Time       : 
 */
sint32 EventBase::event_loop()
{
    return event_base_loop(m_event_base, 0);
}

        
/*
 * Function Descript : 
 * Create Time       : 2010-12-21 19:38
 * Parameter List    : 
 * 
 * Return            : 
 *  0=succ
 *  -1=failed
 * Modify Time       : 
 */
sint32 EventBase::delete_event(Job *job)
{
//    LOG_INFO("delete a job event,the address is:"<<job);
    return event_del(job->get_event());
}

        
/*
 * Function Descript : 
 * Create Time       : 2011-08-02 21:01
 * Parameter List    : 
 *  
 * Return            : 
 *  -1: 失败
 * Modify Time       : 
 */
sint32 EventBase::update_event(Job *job)
{
    struct event *ev = job->get_event();
    struct event_base *base = ev->ev_base;
    if (NULL == base)
    {
        base = m_event_base;
    }
    if (-1 == event_del(ev)) 
    {
//        assert(0);
    }

    sint16 new_flags = job->get_event_flags();
    sint32 fd = job->get_fd() > 0 ? job->get_fd() : -1;
    event_set(ev, fd, new_flags, EventHandler::handler, job);
    event_base_set(base, ev);

    struct timeval *ptv = job->get_timeval();
    
    if (-1 == event_add(ev, ptv))
    {
        return -1;
    }

    return 0;
}

sint32 EventBase::event_loop_exit()
{
    return event_base_loopexit(m_event_base, NULL);
}

}
