#include "htp_sender_job.h"
#include "htp_sender.h"

namespace hpf
{

sint32 HtpSenderJob::run(sint32 fd, sint16 which)
{
    m_htp_sender->pop_asyn_operation(fd);

    return 0;
}

}
