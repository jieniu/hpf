/*
 * filename      : htp_job.h
 * descriptor    :  
 * author        : chenyuhui
 * create time   : 2011-12-03 09:41
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date           | who           | modify summary            |
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_JOB_H_
#define _HTP_JOB_H_

#include "job.h"

namespace hpf
{

class HtpConnectionInfo;

class HtpJob : public Job
{
public:
    HtpJob(HtpConnectionInfo *conn, sint32 fd, sint32 timeout);
    virtual ~HtpJob();

    virtual sint32 run(sint32 fd, sint16 which);
    virtual void delete_event_success();
protected:
    HtpConnectionInfo *m_htp_connection;
};

}
#endif
