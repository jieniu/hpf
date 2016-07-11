#include "htp_sender.h"
#include <fcntl.h>
#include "htp_sender_job.h"
#include "htp_connection_info.h"
#include "recycle_thread.h"
#include "global_var.h"
#include "asyn_operation.h"
#include "base_define.h"
#include "htp_job.h"

namespace hpf
{

class Channel
{
    public: 
        typedef Fifo<AsynOperation *> OperationList;
        Channel()
            : m_write_fd(-1)
        {
        }
        ~Channel()
        {
            if (m_write_fd != -1)
            {
                close(m_write_fd);
                m_write_fd = -1;
            }
        }

        sint32 m_write_fd;
        OperationList m_operation_list;
        HtpSenderJob m_job;
}; 

class HtpSenderCleanHandler : public MessageHandler
{
    public:
        HtpSenderCleanHandler(){}
        virtual ~HtpSenderCleanHandler(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            HtpSender *sender = (HtpSender *)arg1;
            sender->do_clean();
        }
};

HtpSenderCleanHandler HtpSender::s_htp_sender_clean_handler;

HtpSender::HtpSender(sint32 fd, sint32 max_thread_number, HtpConnectionInfo *ci, HtpJob *job)
    : m_fd(fd), m_max_thread_number(max_thread_number), m_is_reconnect(ci->is_reconnect()), 
    m_channels(NULL), m_htp_connection_info(ci), m_job(job)
{
}

HtpSender::~HtpSender()
{
    if (m_channels)
    {
        delete [] m_channels;
        m_channels = NULL;
    }
    m_htp_connection_info = NULL;
    m_job = NULL;
}

sint32 HtpSender::self_init()
{
    m_channels = new Channel[m_max_thread_number]; 
    for (sint32 i = 0; i < m_max_thread_number; ++i)
    {
        int fds[2] = {0};
        if (-1 == pipe(fds))
        {
            LOGGER_ERROR(g_framework_logger, "create pipe error: " << strerror(errno));
            delete [] m_channels;
            m_channels = NULL;
            return -1;
        }

        int flag = fcntl(fds[1], F_GETFL, 0);
        flag |= O_NONBLOCK;
        fcntl(fds[1], F_SETFL, flag);
        m_channels[i].m_job.set_htp_sender(this);
        m_channels[i].m_write_fd = fds[1];
        m_channels[i].m_job.set_fd(fds[0]);
        m_channels[i].m_job.set_event_flags(EV_READ | EV_PERSIST);
        int ret = do_add_job(&(m_channels[i].m_job));
        assert(0 == ret);
    }
    return 0; 
}

void HtpSender::do_clean()
{
    m_fd = -1;
    do_delete_job(m_job); 
    m_job->set_fd(-1);
    for (sint32 i = 0; i < m_max_thread_number; ++i)
    {
        AsynOperation *ao = NULL;
        while (m_channels[i].m_operation_list.pop(ao) != -1)
        {
            ao->on_operation_complete(-2, 0);
        }
    }

    AsynOperation *ao = NULL;
    while (m_send_buffer_list.pop(ao) != -1)
    {
        ao->on_operation_complete(-2, 0);
    }

    for (sint32 i = 0; i < m_max_thread_number; ++i)
    {
        do_delete_job(&m_channels[i].m_job);
    }

    RecycleThread::get_instance()->recycle_thread(this);
}


// 线程进入事件循环前调用
sint32 HtpSender::begin_routine()
{
    m_htp_connection_info->count_active_thread();

    return 0;
}

// 线程停止事件循环前调用
// 最好在这里删除掉所有的事件, 否则可能会崩溃
sint32 HtpSender::end_routine()
{
    m_htp_connection_info->decrease_active_thread();
    return 0;
}

void HtpSender::clean()
{
    post_message(&s_htp_sender_clean_handler, this, NULL);
}

sint32 HtpSender::push_asyn_operation(sint32 no, AsynOperation *ao)
{
    ret_val_if_fail(no < m_max_thread_number, -1);
    if (-1 == m_fd)
    {
        return -1;
    }

    m_channels[no].m_operation_list.push_back(ao);

    char buf = 0;
    if (1 != write(m_channels[no].m_write_fd, &buf, 1) && errno != EAGAIN)
    {
        LOGGER_ERROR(g_framework_logger, "write pipe error: " << strerror(errno)
                     << ",fd:" << m_channels[no].m_write_fd << ",channel:" << no);
        m_channels[no].m_operation_list.erase_latest(ao);
        return -1;
    }

    return 0;
}

// 获取写事件，并向socket写数据
sint32 HtpSender::pop_asyn_operation(sint32 fd)
{
    for (sint32 i = 0; i < m_max_thread_number; ++i)
    {
        if (fd == m_channels[i].m_job.get_fd())
        {
            // TODO mutiple read at once
            char buf = 0;
            if (1 != read(fd, &buf, 1))
            {
                LOGGER_WARN(g_framework_logger, "read fifo failed");
            }

            AsynOperation *ao;
            sint32 ret = 0;
            while (-1 != m_channels[i].m_operation_list.pop(ao))
            {
                sint32 write_len = 0;
                ret = write_data(m_fd, ao, write_len);
                if (-1 == ret)
                {
                    return m_htp_connection_info->close();
                }
            }
            if (0 == ret)
            {
                m_htp_connection_info->update_event();
            }
            return 0;
        }
    }

    LOGGER_WARN(g_framework_logger, "not any channel can be found to match event, fd = " << fd);
    return -1;
}

}
