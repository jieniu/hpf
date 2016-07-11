/*
 * filename      : job.h
 * descriptor    : 
 * author        : fengyajie
 * create time   : 2010-12-24 00:21
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _JOB_H_
#define _JOB_H_

#include "base_header.h"
#include <event.h>
#include "global_var.h"

namespace hpf
{

class HccEventHandler;
class WorkerThread;
class Job
{
    public: 
        Job(sint32 fd)
        {
            m_fd = fd;
            m_worker_thread = 0;
            m_event_flags = 0;
            m_event = (struct event*)malloc(sizeof(event));
            memset((void*)m_event, 0, sizeof(event));
            memset(&m_timeval, 0, sizeof(m_timeval));
        }

        virtual ~Job() 
        {
            free(m_event);
            m_event = NULL;
            m_worker_thread = NULL;
            if (-1 != m_fd)
            {
                close(m_fd);
                m_fd = -1;
            }
        }

        virtual sint32 run(sint32 fd, sint16 which) = 0;

        // call this after delete from event_base
        virtual void delete_event_success() = 0;

        sint32 get_fd() {return m_fd;}
        void set_fd(sint32 fd) {m_fd = fd;}

        struct event *get_event() {return m_event;}

        sint16 get_event_flags() {return m_event_flags;}
        void set_event_flags(sint16 ef) {m_event_flags = ef;}

        void set_worker_thread(WorkerThread *wt) {m_worker_thread = wt;}
        WorkerThread *get_worker_thread() {return m_worker_thread;}

        timeval* get_timeval(); 
    protected:
        sint32 m_fd;
        struct event *m_event;
        sint16 m_event_flags;
        WorkerThread *m_worker_thread;
        timeval m_timeval; // timeoutÊÂ¼þ
};

inline timeval* Job::get_timeval()
{
    if (m_timeval.tv_sec > 0 || m_timeval.tv_usec > 0)
    {
        return &m_timeval;
    }

    return NULL;
}

}
#endif

