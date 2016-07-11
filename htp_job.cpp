#include "htp_job.h"
#include "htp_connection_info.h"

namespace hpf
{

HtpJob::HtpJob(HtpConnectionInfo *conn, sint32 fd, sint32 timeout)
    : Job(fd), m_htp_connection(conn)
{
    m_timeval.tv_sec = timeout;
    m_timeval.tv_usec = 0;
}

HtpJob::~HtpJob()
{
    m_htp_connection = NULL;
}

sint32 HtpJob::run(sint32 fd, sint16 which)
{
    return m_htp_connection->run(fd, which);
}

void HtpJob::delete_event_success()
{
}

}

