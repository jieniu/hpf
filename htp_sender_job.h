/*
 * filename      : htp_sender_job.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 21:08
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HTP_SENDER_JOB_H_
#define _HTP_SENDER_JOB_H_
#include "job.h"

namespace hpf
{

class HtpSender;
class HtpSenderJob : public Job
{
    public:   
        HtpSenderJob()
            : Job(-1), m_htp_sender(NULL)
        {}
        virtual ~HtpSenderJob(){}

        virtual sint32 run(sint32 fd, sint16 which);
        
        virtual void delete_event_success() {}

        void set_htp_sender(HtpSender *hs) {m_htp_sender = hs;}

    private:
        HtpSender *m_htp_sender;
};

}
#endif
