/*
 * filename      : timeout_job.h
 * descriptor    :
 * author        : fengyajie
 * create time   : 2011-08-04 13:50
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _TIMEOUT_JOB_H_
#define _TIMEOUT_JOB_H_
#include "base_header.h"
#include "job.h"

namespace hpf
{

class Reactor;
class TimeoutHandler
{
    public:
        TimeoutHandler(){}
        virtual ~TimeoutHandler(){}

        //对于循环定时器，返回false表示取消定时器
        virtual bool handle_timeout(uint32 handle, void *arg) = 0;
};

class TimeoutJob : public Job
{
    public:
        TimeoutJob(TimeoutHandler *handler, void *arg, const timeval &tv,
                sint32 rotate, sint32 handle);
        virtual ~TimeoutJob(){}

        virtual sint32 run(sint32 fd, sint16 which);

        virtual void delete_event_success() {delete this;}
    private:
        TimeoutHandler *m_handler;
        void *m_arg;
        sint32 m_rotate;
        sint32 m_handle;
};

inline
TimeoutJob::TimeoutJob(TimeoutHandler *handler, void *arg, const timeval &tv,
                       sint32 rotate, sint32 handle)
    : Job(-1), m_handler(handler), m_arg(arg), m_rotate(rotate), m_handle(handle)
{
    m_timeval.tv_sec = tv.tv_sec;
    m_timeval.tv_usec = tv.tv_usec;
}

class TimeoutJobUint : public Job
{
    public:
        TimeoutJobUint(TimeoutHandler *handler, uint32 arg, const timeval &tv,
                sint32 rotate, sint32 handle);
        virtual ~TimeoutJobUint(){}

        virtual sint32 run(sint32 fd, sint16 which);

        virtual void delete_event_success() {delete this;}
    private:
        TimeoutHandler *m_handler;
        uint32 m_arg;
        sint32 m_rotate;
        sint32 m_handle;
};

inline
TimeoutJobUint::TimeoutJobUint(TimeoutHandler *handler, uint32 arg, const timeval &tv,
                               sint32 rotate, sint32 handle)
    : Job(-1), m_handler(handler), m_arg(arg), m_rotate(rotate), m_handle(handle)
{
    m_timeval.tv_sec = tv.tv_sec;
    m_timeval.tv_usec = tv.tv_usec;
}
}
#endif
