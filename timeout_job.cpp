#include "timeout_job.h"
#include "reactor.h"
#include "base_define.h"
#include "worker_thread.h"

namespace hpf
{

sint32 TimeoutJob::run(sint32 fd, sint16 which)
{
    assert(which == EV_TIMEOUT);

    ret_val_if_fail(m_handler, -1);

    bool bcontinue = m_handler->handle_timeout(m_handle, m_arg);

    if (0 == m_rotate || !bcontinue)
    {
        Reactor::get_instance()->cancel_timer(m_handle);
        return 0;
    }

    // 在所在线程中，直接增加
    m_worker_thread->do_add_job(this);

    return 0;
}

sint32 TimeoutJobUint::run(sint32 fd, sint16 which)
{
    assert(which == EV_TIMEOUT);

    ret_val_if_fail(m_handler, -1);

    bool bcontinue = m_handler->handle_timeout(m_handle, &m_arg);

    if (0 == m_rotate || !bcontinue)
    {
        Reactor::get_instance()->cancel_timer(m_handle);
        return 0;
    }

    // 在所在线程中，直接增加
    m_worker_thread->do_add_job(this);

    return 0;
}
}
