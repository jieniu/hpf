/*
 * filename      : reactor.h
 * descriptor    :
 * author        : fengyajie
 * create time   : 2010-12-22 20:09
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * | 8.4            |fengyajie      | 整理头文件                |
 * +----------------+---------------+---------------------------+
 */
#ifndef _REACTOR_H_
#define _REACTOR_H_

#include "base_header.h"

namespace hpf
{


class ListenThread;
class WorkerThreadPool;
class ConnectionHandler;
class MessageProcessor;
class TimeoutHandler;
class Job;
class MessageParser;
class MessageHandler;
//class HttpRequest;
//class HttpResponseCallBack;

class Reactor
{
    public:
        typedef vector<ListenThread*> ListenThreads;
        typedef map<sint32, Job*> Timers;

        sint32 init(sint32 num_thread, sint32 sec);

        sint32 add_hcc_listener(const char *ip, uint16 port,
                MessageProcessor *mp, sint32 timeout);

        sint32 add_connector(const char *ip, uint16 port,
                MessageProcessor *mp, bool is_reconnect);

        sint32 add_htp_connector(const char *ip, uint16 port, MessageParser *parser,
                MessageProcessor *mp, sint32 is_reconnect);

        sint32 add_htp_listener(const char *ip, uint16 port, MessageParser *parser,
                MessageProcessor *mp, sint32 timeout = 0);

        sint32 add_timer(TimeoutHandler *th, void* arg, const timeval &tv,
                sint32 rotate, uint32 &handle);

        sint32 add_timer(TimeoutHandler *th, uint32 arg, const timeval &tv,
                sint32 rotate, uint32 &handle);

        void cancel_timer(sint32 handle);

        sint32 create_udp_server(const char *ip, uint16 port, MessageProcessor *mp);

        sint32 post_message(MessageHandler *handler, void *arg1, void *arg2);
        sint32 post_message(MessageHandler *handler, void *arg1, void *arg2, pthread_t id);

        static Reactor *get_instance();

        void destroy();

        sint32 get_wthread_number();

        const vector<pthread_t> &get_thread_ids();
        sint32 get_thread_pos(pthread_t id);

    private:

        Reactor();
        ~Reactor();

        WorkerThreadPool *m_worker_thread_pool;
        uint32 m_seqid;
        pthread_mutex_t m_mutex;
        ListenThreads m_listen_threads;
        Timers m_timers;
        static Reactor *s_instance;
};
}
#endif
